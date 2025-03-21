#pragma once

#include "di/execution/concepts/prelude.h"
#include "di/execution/interface/get_env.h"
#include "di/execution/interface/prelude.h"
#include "di/execution/meta/env_of.h"
#include "di/execution/meta/prelude.h"
#include "di/execution/query/get_scheduler.h"
#include "di/execution/query/make_env.h"
#include "di/execution/query/prelude.h"
#include "di/execution/receiver/prelude.h"
#include "di/execution/types/prelude.h"
#include "di/function/tag_invoke.h"
#include "di/util/declval.h"
#include "di/util/defer_construct.h"

namespace di::execution {
namespace on_ns {
    template<typename Send, typename Rec, typename Sched>
    struct OperationStateT {
        struct Type;
    };

    template<concepts::Sender Send, concepts::Receiver Rec, concepts::Scheduler Sched>
    using OperationState = meta::Type<OperationStateT<Send, Rec, Sched>>;

    template<typename Base, concepts::Scheduler Sched>
    using Env = MakeEnv<Base, With<types::Tag<get_scheduler>, Sched>>;

    template<typename Send, typename Rec, typename Sched>
    struct ReceiverWithEnvT {
        struct Type : ReceiverAdaptor<Type> {
        private:
            using Base = ReceiverAdaptor<Type>;
            friend Base;

        public:
            explicit Type(OperationState<Send, Rec, Sched>* operation_state) : m_operation_state(operation_state) {}

            auto base() const& -> Rec const& { return m_operation_state->receiver; }
            auto base() && -> Rec&& { return util::move(m_operation_state->receiver); }

        private:
            auto get_env() const& -> Env<meta::EnvOf<Rec>, Sched> {
                return make_env(execution::get_env(base()), with(get_scheduler, m_operation_state->scheduler));
            }

            OperationState<Send, Rec, Sched>* m_operation_state;
        };
    };

    template<concepts::Sender Send, concepts::Receiver Rec, concepts::Scheduler Sched>
    using ReceiverWithEnv = meta::Type<ReceiverWithEnvT<Send, Rec, Sched>>;

    template<typename Send, typename Rec, typename Sched>
    struct ReceiverT {
        struct Type : ReceiverAdaptor<Type> {
        private:
            using Base = ReceiverAdaptor<Type>;
            friend Base;

        public:
            explicit Type(OperationState<Send, Rec, Sched>* operation_state) : m_operation_state(operation_state) {}

            auto base() const& -> Rec const& { return m_operation_state->receiver; }
            auto base() && -> Rec&& { return util::move(m_operation_state->receiver); }

        private:
            void set_value() && { m_operation_state->phase2(); }

            OperationState<Send, Rec, Sched>* m_operation_state;
        };
    };

    template<concepts::Sender Send, concepts::Receiver Rec, concepts::Scheduler Sched>
    using Receiver = meta::Type<ReceiverT<Send, Rec, Sched>>;

    template<typename Send, typename Rec, typename Sched>
    struct OperationStateT<Send, Rec, Sched>::Type : util::Immovable {
    public:
        [[no_unique_address]] Sched scheduler;
        [[no_unique_address]] Send sender;
        [[no_unique_address]] Rec receiver;
        Variant<meta::ConnectResult<meta::ScheduleResult<Sched>, Receiver<Send, Rec, Sched>>,
                meta::ConnectResult<Send, ReceiverWithEnv<Send, Rec, Sched>>>
            operation_state;

        template<typename S>
        requires(concepts::ConstructibleFrom<Send, S>)
        explicit Type(Sched scheduler_, S&& sender_, Rec receiver_)
            : scheduler(util::move(scheduler_))
            , sender(util::forward<S>(sender_))
            , receiver(util::move(receiver_))
            , operation_state(c_<0ZU>, util::DeferConstruct([&] {
                                  return execution::connect(execution::schedule(scheduler),
                                                            Receiver<Send, Rec, Sched> { this });
                              })) {}

        void phase2() {
            operation_state.template emplace<1>(util::DeferConstruct([&] {
                return execution::connect(util::move(sender), ReceiverWithEnv<Send, Rec, Sched> { this });
            }));
            execution::start(util::get<1>(operation_state));
        }

    private:
        friend void tag_invoke(types::Tag<execution::start>, Type& self) {
            execution::start(util::get<0>(self.operation_state));
        }
    };

    template<typename Send, typename Sched>
    struct SenderT {
        struct Type {
            using is_sender = void;

            [[no_unique_address]] Sched scheduler;
            [[no_unique_address]] Send sender;

        private:
            template<concepts::DecaysTo<Type> Self, typename Rec>
            requires(concepts::DecayConstructible<meta::Like<Self, Send>> &&
                     concepts::SenderTo<meta::Like<Self, Send>, ReceiverWithEnv<Send, Rec, Sched>>)
            friend auto tag_invoke(types::Tag<connect>, Self&& self, Rec receiver) {
                return OperationState<Send, Rec, Sched> { util::forward_like<Self>(self.scheduler),
                                                          util::forward_like<Self>(self.sender), util::move(receiver) };
            }

            template<concepts::DecaysTo<Type> Self, typename E>
            friend auto tag_invoke(types::Tag<get_completion_signatures>, Self&&, E&&)
                -> meta::MakeCompletionSignatures<
                    meta::Like<Self, Send>, Env<E, Sched>,
                    meta::MakeCompletionSignatures<meta::ScheduleResult<Sched>, MakeEnv<E>, CompletionSignatures<>,
                                                   meta::TypeConstant<CompletionSignatures<>>::template Invoke>> {
                return {};
            }

            constexpr friend auto tag_invoke(types::Tag<get_env>, Type const& self) {
                return make_env(get_env(self.sender));
            }
        };
    };

    template<concepts::Sender Send, concepts::Scheduler Sched>
    using Sender = meta::Type<SenderT<Send, Sched>>;

    struct Function {
        template<concepts::Scheduler Sched, concepts::Sender Send>
        auto operator()(Sched&& scheduler, Send&& sender) const -> concepts::Sender auto {
            if constexpr (concepts::TagInvocable<Function, Sched, Send>) {
                return function::tag_invoke(*this, util::forward<Sched>(scheduler), util::forward<Send>(sender));
            } else {
                return Sender<meta::Decay<Send>, meta::Decay<Sched>> { util::forward<Sched>(scheduler),
                                                                       util::forward<Send>(sender) };
            }
        }
    };
}

/// execution::on() takes a scheduler and sender, and returns a new sender
/// whose which "runs" the provided sender on designated scheduler.
///
/// This is implemented by "connect"ing to the result of execution::schedule(),
/// and only starting the provided sender when that completes. Additionally,
/// execution::on() wraps any provided receivers with a new enviornment which
/// adverties the scheduler for execution::get_scheduler().
constexpr inline auto on = on_ns::Function {};
}
