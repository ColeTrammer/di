#pragma once

#include "di/container/concepts/prelude.h"
#include "di/container/meta/prelude.h"
#include "di/function/compare.h"
#include "di/function/identity.h"

namespace di::util {
namespace detail {
    struct ClampFunction {
        template<typename T, typename Proj = function::Identity,
                 concepts::IndirectStrictWeakOrder<meta::Projected<T const*, Proj>> Comp = function::Compare>
        constexpr auto operator()(T const& value, T const& min, T const& max, Comp comp = {}, Proj proj = {}) const
            -> T const& {
            auto&& projected_value = function::invoke(proj, value);

            // NOLINTBEGIN(bugprone-return-const-ref-from-parameter)
            if (function::invoke(comp, projected_value, function::invoke(proj, min)) < 0) {
                return min;
            }
            if (function::invoke(comp, projected_value, function::invoke(proj, max)) > 0) {
                return max;
            }
            return value;
            // NOLINTEND(bugprone-return-const-ref-from-parameter)
        }
    };
}

constexpr inline auto clamp = detail::ClampFunction {};
}

namespace di {
using util::clamp;
}
