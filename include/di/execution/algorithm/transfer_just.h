#pragma once

#include "di/execution/algorithm/just.h"
#include "di/execution/algorithm/transfer.h"
#include "di/execution/concepts/sender.h"
#include "di/meta/util.h"

namespace di::execution {
namespace transfer_just_ns {
    struct Function {
        template<concepts::Scheduler Sched, concepts::MovableValue... Values>
        auto operator()(Sched&& scheduler, Values&&... values) const -> concepts::Sender auto {
            if constexpr (concepts::TagInvocable<Function, Sched, Values...>) {
                return function::tag_invoke(*this, util::forward<Sched>(scheduler), util::forward<Values>(values)...);
            } else {
                return execution::transfer(execution::just(util::forward<Values>(values)...),
                                           util::forward<Sched>(scheduler));
            }
        }
    };
}

constexpr inline auto transfer_just = transfer_just_ns::Function {};
}
