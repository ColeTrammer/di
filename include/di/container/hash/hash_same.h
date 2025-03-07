#pragma once

#include "di/container/hash/default_hasher.h"
#include "di/container/hash/hash_write.h"
#include "di/function/tag_invoke.h"
#include "di/function/unpack.h"
#include "di/meta/algorithm.h"
#include "di/meta/core.h"
#include "di/meta/language.h"
#include "di/types/prelude.h"
#include "di/vocab/tuple/tuple_element.h"
#include "di/vocab/tuple/tuple_like.h"
#include "di/vocab/tuple/tuple_size.h"

namespace di::container {
namespace detail {
    struct HashSameFunction {
        template<typename T, typename U>
        constexpr auto operator()(InPlaceType<T>, InPlaceType<U>) const -> bool {
            return concepts::SameAs<T, U>;
        }

        template<concepts::IntegralOrEnum T, concepts::IntegralOrEnum U>
        requires(!concepts::TagInvocable<HashWriteFunction, DefaultHasher&, T> &&
                 !concepts::TagInvocable<HashWriteFunction, DefaultHasher&, U>)
        constexpr auto operator()(InPlaceType<T>, InPlaceType<U>) const -> bool {
            return sizeof(T) == sizeof(U);
        }

        template<typename T, typename U>
        requires(concepts::TagInvocableTo<HashSameFunction, bool, InPlaceType<T>, InPlaceType<U>>)
        constexpr auto operator()(InPlaceType<T>, InPlaceType<U>) const -> bool {
            return function::tag_invoke(*this, in_place_type<T>, in_place_type<U>);
        }
    };
}

constexpr inline auto hash_same = detail::HashSameFunction {};
}

namespace di::concepts {
template<typename T, typename U>
concept HashSame = Hashable<T> && Hashable<U> &&
                   container::hash_same(in_place_type<meta::RemoveCVRef<T>>, in_place_type<meta::RemoveCVRef<U>>);
}

namespace di::container::detail {
template<HashableContainer T, HashableContainer U>
constexpr auto tag_invoke(types::Tag<hash_same>, InPlaceType<T>, InPlaceType<U>) -> bool {
    return concepts::HashSame<meta::ContainerReference<T>, meta::ContainerReference<U>>;
}

template<concepts::TupleLike T, concepts::TupleLike U>
requires((meta::TupleSize<T> == meta::TupleSize<U>) && !HashableContainer<T> && !HashableContainer<U>)
constexpr auto tag_invoke(types::Tag<hash_same>, InPlaceType<T>, InPlaceType<U>) -> bool {
    return function::unpack<meta::MakeIndexSequence<meta::TupleSize<T>>>(
        [&]<usize... indices>(meta::ListV<indices...>) {
            return (concepts::HashSame<meta::TupleElement<T, indices>, meta::TupleElement<U, indices>> && ...);
        });
}
}

namespace di {
using concepts::HashSame;
}
