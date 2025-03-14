#pragma once

#include "di/any/concepts/method.h"
#include "di/any/meta/method_tag.h"
#include "di/any/types/method.h"
#include "di/any/types/prelude.h"
#include "di/function/invoke.h"
#include "di/function/tag_invoke.h"
#include "di/meta/util.h"

namespace di::concepts {
namespace detail {
    template<typename M, typename T>
    constexpr inline bool method_callable_with_helper = false;

    template<typename Tag, typename R, concepts::RemoveCVRefSameAs<This> Self, typename... BArgs, typename T>
    constexpr inline bool method_callable_with_helper<types::Method<Tag, R(Self, BArgs...)>, T> =
        TagInvocableTo<Tag const&, R, types::Method<Tag, R(Self, BArgs...)>, meta::Like<Self, T>, BArgs...> ||
        InvocableTo<Tag const&, R, meta::Like<Self, T>, BArgs...>;
}

template<typename M, typename T>
concept MethodCallableWith = Method<M> && detail::method_callable_with_helper<M, T>;
}
