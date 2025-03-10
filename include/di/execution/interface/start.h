#pragma once

#include "di/any/types/method.h"
#include "di/any/types/this.h"
#include "di/function/tag_invoke.h"

namespace di::execution {
namespace detail {
    struct StartFunction {
        using Type = types::Method<StartFunction, void(types::This&)>;

        template<typename OpState>
        requires(concepts::TagInvocable<StartFunction, OpState&>)
        constexpr auto operator()(OpState& op_state) const {
            return function::tag_invoke(*this, op_state);
        }
    };
}

constexpr inline auto start = detail::StartFunction {};
}
