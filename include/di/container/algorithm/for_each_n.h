#pragma once

#include "di/container/algorithm/in_fun_result.h"
#include "di/container/concepts/prelude.h"
#include "di/container/meta/prelude.h"
#include "di/function/identity.h"
#include "di/function/invoke.h"
#include "di/util/reference_wrapper.h"

namespace di::container {
namespace detail {
    struct ForEachNFunction {
        template<concepts::InputIterator Iter, typename SSizeType = meta::IteratorSSizeType<Iter>,
                 typename Proj = function::Identity, concepts::IndirectlyUnaryInvocable<meta::Projected<Iter, Proj>> F>
        constexpr auto operator()(Iter first, meta::TypeIdentity<SSizeType> n, F f, Proj proj = {}) const
            -> InFunResult<Iter, F> {
            for (SSizeType i = 0; i < n; ++i, ++first) {
                function::invoke(f, function::invoke(proj, *first));
            }
            return { util::move(first), util::move(f) };
        }
    };
}

constexpr inline auto for_each_n = detail::ForEachNFunction {};
}

namespace di {
using container::for_each_n;
}
