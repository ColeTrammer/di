#pragma once

#include "di/container/algorithm/in_out_result.h"
#include "di/container/concepts/prelude.h"
#include "di/container/iterator/prelude.h"
#include "di/container/meta/prelude.h"

namespace di::container {
namespace detail {
    struct CopyIfFunction {
        template<concepts::InputIterator It, concepts::SentinelFor<It> Sent, concepts::WeaklyIncrementable Out,
                 typename Proj = function::Identity, concepts::IndirectUnaryPredicate<meta::Projected<It, Proj>> Pred>
        requires(concepts::IndirectlyCopyable<It, Out>)
        constexpr auto operator()(It first, Sent last, Out output, Pred pred, Proj proj = {}) const
            -> InOutResult<It, Out> {
            for (; first != last; ++first) {
                if (function::invoke(pred, function::invoke(proj, *first))) {
                    *output = *first;
                    ++output;
                }
            }
            return { util::move(first), util::move(output) };
        }

        template<concepts::InputContainer Con, concepts::WeaklyIncrementable Out, typename Proj = function::Identity,
                 concepts::IndirectUnaryPredicate<meta::Projected<meta::ContainerIterator<Con>, Proj>> Pred>
        requires(concepts::IndirectlyCopyable<meta::ContainerIterator<Con>, Out>)
        constexpr auto operator()(Con&& container, Out output, Pred pred, Proj proj = {}) const
            -> InOutResult<meta::BorrowedIterator<Con>, Out> {
            return (*this)(container::begin(container), container::end(container), util::move(output), util::ref(pred),
                           util::ref(proj));
        }
    };
}

constexpr inline auto copy_if = detail::CopyIfFunction {};
}

namespace di {
using container::copy_if;
}
