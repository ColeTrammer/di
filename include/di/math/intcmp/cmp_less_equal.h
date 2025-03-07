#pragma once

#include "di/math/intcmp/cmp_greater.h"

namespace di::math {
namespace detail {
    struct CmpLessEqualFunction {
        template<concepts::Integer T, concepts::Integer U>
        constexpr auto operator()(T a, U b) const -> bool {
            return !cmp_greater(a, b);
        }
    };
}

constexpr inline auto cmp_less_equal = detail::CmpLessEqualFunction {};
}
