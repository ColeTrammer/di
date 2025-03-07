#pragma once

#include "di/execution/concepts/awaitable_sender.h"
#include "di/execution/concepts/forwarding_query.h"
#include "di/execution/interface/get_env.h"
#include "di/execution/meta/connect_result.h"
#include "di/execution/meta/env_of.h"
#include "di/execution/meta/single_sender_value_type.h"
#include "di/meta/core.h"

namespace di::execution {
namespace as_awaitable_ns {
    template<typename Send, typename Promise>
    struct AwaitableReceiver<Send, Promise>::Type {
        using is_receiver = void;

        using Value = meta::SingleSenderValueType<Send, meta::EnvOf<Promise>>;
        using Result = meta::Conditional<concepts::LanguageVoid<Value>, Void, Value>;

        Optional<Result>* result_pointer;
        CoroutineHandle<Promise> continuation;

    private:
        template<typename... Args>
        requires(concepts::ConstructibleFrom<Result, Args...>)
        friend void tag_invoke(SetValue, Type&& self, Args&&... args) {
            self.result_pointer->emplace(util::forward<Args>(args)...);
            self.continuation.resume();
        }

        friend void tag_invoke(SetError, Type&& self, Error error) {
            static_cast<CoroutineHandle<>>(self.continuation.promise().unhandled_error(util::move(error))).resume();
        }

        friend void tag_invoke(SetStopped, Type&& self) {
            static_cast<CoroutineHandle<>>(self.continuation.promise().unhandled_stopped()).resume();
        }

        constexpr friend auto tag_invoke(types::Tag<get_env> tag, Type const& self) -> decltype(auto) {
            return tag(self.continuation.promise());
        }
    };

    template<typename Send, typename Promise>
    struct SenderAwaitableT {
        struct Type {
        private:
            using Receiver = meta::Type<AwaitableReceiver<Send, Promise>>;
            using Value = meta::SingleSenderValueType<Send, meta::EnvOf<Promise>>;
            using Result = meta::Conditional<concepts::LanguageVoid<Value>, Void, Value>;

        public:
            explicit Type(Send&& sender, Promise& promise)
                : m_state(connect(
                      util::forward<Send>(sender),
                      Receiver { util::addressof(m_result), CoroutineHandle<Promise>::from_promise(promise) })) {}

            auto await_ready() const noexcept -> bool { return false; }
            void await_suspend(CoroutineHandle<>) noexcept { start(m_state); }
            auto await_resume() -> Value {
                if constexpr (!concepts::LanguageVoid<Value>) {
                    return util::move(m_result).value();
                }
            }

        private:
            Optional<Result> m_result {};
            meta::ConnectResult<Send, Receiver> m_state;
        };
    };

    template<typename Send, typename Promise>
    using SenderAwaitable = meta::Type<SenderAwaitableT<Send, Promise>>;

    struct DummyPromise {
        auto get_return_object() noexcept -> DummyPromise;
        auto initial_suspend() noexcept -> SuspendAlways;
        auto final_suspend() noexcept -> SuspendAlways;
        void unhandled_exception() noexcept;
        void return_void() noexcept;

        auto unhandled_stopped() noexcept -> std::coroutine_handle<>;
        auto unhandled_error(vocab::Error) noexcept -> std::coroutine_handle<>;
    };

    struct Function {
        template<typename T, typename Promise>
        constexpr auto operator()(T&& value, Promise& promise) const -> decltype(auto) {
            if constexpr (concepts::TagInvocable<Function, T, Promise&>) {
                static_assert(concepts::IsAwaitable<meta::TagInvokeResult<Function, T, Promise&>>,
                              "Customizations of di::as_awaitable() must return an Awaitable.");
                return function::tag_invoke(*this, util::forward<T>(value), promise);
            } else if constexpr (concepts::IsAwaitable<T, DummyPromise>) {
                return util::forward<T>(value);
            } else if constexpr (concepts::AwaitableSender<T, Promise>) {
                return SenderAwaitable<T, Promise> { util::forward<T>(value), promise };
            }
        }
    };
}

constexpr inline as_awaitable_ns::Function as_awaitable = {};
}

namespace di {
using execution::as_awaitable;
}
