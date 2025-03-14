#pragma once

#include "di/function/tag_invoke.h"
#include "di/meta/core.h"
#include "di/types/in_place_type.h"
#include "di/types/size_t.h"

namespace di::vocab {
struct TupleSizeFunction {
    template<typename T>
    requires(concepts::TagInvocableTo<TupleSizeFunction, types::size_t, types::InPlaceType<T>>)
    constexpr auto operator()(types::InPlaceType<T>) const -> types::size_t {
        return function::tag_invoke(*this, types::in_place_type<T>);
    }
};

constexpr inline auto tuple_size = TupleSizeFunction {};
}

namespace di::meta {
template<typename T>
requires(requires { vocab::tuple_size(types::in_place_type<meta::RemoveCVRef<T>>); })
constexpr inline auto TupleSize = vocab::tuple_size(types::in_place_type<meta::RemoveCVRef<T>>);
}
