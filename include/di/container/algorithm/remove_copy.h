#pragma once

#include "di/container/algorithm/in_out_result.h"
#include "di/container/concepts/prelude.h"
#include "di/container/iterator/prelude.h"
#include "di/container/meta/prelude.h"
#include "di/function/equal.h"

namespace di::container {
namespace detail {
    struct RemoveCopyFunction {
        template<concepts::InputIterator It, concepts::SentinelFor<It> Sent, concepts::WeaklyIncrementable Out,
                 typename T, typename Proj = function::Identity>
        requires(concepts::IndirectlyCopyable<It, Out> &&
                 concepts::IndirectBinaryPredicate<function::Equal, meta::Projected<It, Proj>, T const*>)
        constexpr auto operator()(It first, Sent last, Out output, T const& value, Proj proj = {}) const
            -> InOutResult<It, Out> {
            for (; first != last; ++first) {
                if (value != function::invoke(proj, *first)) {
                    *output = *first;
                    ++output;
                }
            }
            return { util::move(first), util::move(output) };
        }

        template<concepts::ForwardContainer Con, concepts::WeaklyIncrementable Out, typename T,
                 typename Proj = function::Identity>
        requires(concepts::IndirectlyCopyable<meta::ContainerIterator<Con>, Out> &&
                 concepts::IndirectBinaryPredicate<function::Equal, meta::Projected<meta::ContainerIterator<Con>, Proj>,
                                                   T const*>)
        constexpr auto operator()(Con&& container, Out output, T const& value, Proj proj = {}) const
            -> InOutResult<meta::BorrowedIterator<Con>, Out> {
            return (*this)(container::begin(container), container::end(container), util::move(output), value,
                           util::ref(proj));
        }
    };
}

constexpr inline auto remove_copy = detail::RemoveCopyFunction {};
}

namespace di {
using container::remove_copy;
}
