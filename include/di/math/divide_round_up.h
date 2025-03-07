#pragma once

#include "di/function/curry_back.h"
#include "di/meta/language.h"
#include "di/meta/util.h"

namespace di::math {
namespace detail {
    struct DivideRoundUpFunction {
        template<concepts::Integer T>
        constexpr auto operator()(T a, meta::TypeIdentity<T> b) const -> T {
            return (a + b - 1) / b;
        }
    };
}

constexpr inline auto divide_round_up = function::curry_back(detail::DivideRoundUpFunction {}, meta::c_<2ZU>);
}

namespace di {
using math::divide_round_up;
}
