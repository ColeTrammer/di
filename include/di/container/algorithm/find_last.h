#pragma once

#include "di/container/algorithm/find.h"
#include "di/container/concepts/prelude.h"
#include "di/container/meta/prelude.h"
#include "di/container/view/reverse.h"
#include "di/function/equal.h"
#include "di/function/identity.h"
#include "di/function/invoke.h"
#include "di/util/reference_wrapper.h"

namespace di::container {
namespace detail {
    struct FindLastFunction {
        template<concepts::InputIterator Iter, concepts::SentinelFor<Iter> Sent, typename T,
                 typename Proj = function::Identity>
        requires(concepts::IndirectBinaryPredicate<function::Equal, meta::Projected<Iter, Proj>, T const*>)
        constexpr auto operator()(Iter first, Sent last, T const& needle, Proj proj = {}) const -> View<Iter> {
            if constexpr (concepts::BidirectionalIterator<Iter> && concepts::SameAs<Iter, Sent>) {
                auto rlast = make_reverse_iterator(first);
                auto it = container::find(make_reverse_iterator(last), rlast, needle, util::ref(proj));
                if (it == rlast) {
                    return { last, last };
                }
                return { container::prev(it.base()), last };
            } else {
                Iter result {};
                for (; first != last; ++first) {
                    if (function::invoke(proj, *first) == needle) {
                        result = first;
                    }
                }
                return { result == Iter {} ? first : result, first };
            }
        }

        template<concepts::InputContainer Con, typename T, typename Proj = function::Identity>
        requires(concepts::IndirectBinaryPredicate<function::Equal, meta::Projected<meta::ContainerIterator<Con>, Proj>,
                                                   T const*>)
        constexpr auto operator()(Con&& container, T const& needle, Proj proj = {}) const -> meta::BorrowedView<Con> {
            return (*this)(container::begin(container), container::end(container), needle, util::ref(proj));
        }
    };
}

constexpr inline auto find_last = detail::FindLastFunction {};
}

namespace di {
using container::find_last;
}
