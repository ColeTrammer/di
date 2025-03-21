#pragma once

#include "di/container/concepts/prelude.h"
#include "di/container/meta/prelude.h"
#include "di/function/compare.h"
#include "di/function/identity.h"
#include "di/util/move.h"
#include "di/util/reference_wrapper.h"

namespace di::container {
namespace detail {
    struct IsSortedUntilFunction {
        template<concepts::ForwardIterator Iter, concepts::SentinelFor<Iter> Sent, typename Proj = function::Identity,
                 concepts::IndirectStrictWeakOrder<meta::Projected<Iter, Proj>> Comp = function::Compare>
        constexpr auto operator()(Iter it, Sent last, Comp comp = {}, Proj proj = {}) const -> Iter {
            if (it == last) {
                return it;
            }

            auto next = it;
            while (++next != last) {
                if (function::invoke(comp, function::invoke(proj, *it), function::invoke(proj, *next)) > 0) {
                    return next;
                }
                it = next;
            }
            return next;
        }

        template<concepts::ForwardContainer Con, typename Proj = function::Identity,
                 concepts::IndirectStrictWeakOrder<meta::Projected<meta::ContainerIterator<Con>, Proj>> Comp =
                     function::Compare>
        constexpr auto operator()(Con&& container, Comp comp = {}, Proj proj = {}) const
            -> meta::BorrowedIterator<Con> {
            return (*this)(container::begin(container), container::end(container), util::ref(comp), util::ref(proj));
        }
    };
}

constexpr inline auto is_sorted_until = detail::IsSortedUntilFunction {};
}

namespace di {
using container::is_sorted_until;
}
