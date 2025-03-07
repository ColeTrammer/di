#pragma once

#include "di/container/concepts/prelude.h"
#include "di/container/interface/prelude.h"
#include "di/container/iterator/next.h"
#include "di/container/meta/prelude.h"
#include "di/function/equal.h"

namespace di::container {
namespace detail {
    struct SearchNFunction {
        template<concepts::ForwardIterator It, concepts::SentinelFor<It> Sent, typename T,
                 typename Pred = function::Equal, typename Proj = function::Identity,
                 typename SSizeType = meta::IteratorSSizeType<It>>
        requires(concepts::IndirectlyComparable<It, T const*, Pred, Proj>)
        constexpr auto operator()(It first, Sent last, meta::TypeIdentity<SSizeType> n, T const& value, Pred pred = {},
                                  Proj proj = {}) const -> View<It> {
            if (n <= 0) {
                return { first, first };
            }
            for (; first != last; ++first) {
                if (!function::invoke(pred, function::invoke(proj, *first), value)) {
                    continue;
                }

                // Have one match, try to keep going.
                auto* start = first;
                SSizeType count = 1;
                for (;; ++count) {
                    if (count == n) {
                        return { start, container::next(first) };
                    }
                    if (++first == last) {
                        return { first, first };
                    }
                    if (!function::invoke(pred, function::invoke(proj, *first), value)) {
                        break;
                    }
                }
            }
            return { first, first };
        }

        template<concepts::ForwardContainer Con, typename T, typename Pred = function::Equal,
                 typename Proj = function::Identity>
        requires(concepts::IndirectlyComparable<meta::ContainerIterator<Con>, T const*, Pred, Proj>)
        constexpr auto operator()(Con&& container, meta::ContainerSSizeType<Con> n, T const& needle, Pred pred = {},
                                  Proj proj = {}) const -> meta::BorrowedView<Con> {
            return (*this)(container::begin(container), container::end(container), n, needle, util::ref(pred),
                           util::ref(proj));
        }
    };
}

constexpr inline auto search_n = detail::SearchNFunction {};
}

namespace di {
using container::search_n;
}
