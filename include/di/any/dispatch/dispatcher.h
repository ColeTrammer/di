#pragma once

#include "di/any/types/prelude.h"
#include "di/function/invoke.h"
#include "di/function/tag_invoke.h"
#include "di/meta/algorithm.h"
#include "di/meta/constexpr.h"
#include "di/meta/util.h"
#include "di/util/forward_like.h"

namespace di::any {
namespace detail {
    template<typename Tag, typename Sig, typename... Tags>
    struct DispatcherImpl;

    template<typename Tag, typename R, concepts::DecaysTo<This> Self, typename... Args, typename... Tags>
    struct DispatcherImpl<Tag, R(Self, Args...), Tags...> {
        using Type = Method<Tag, R(Self, Args...)>;

        template<typename T, typename F>
        constexpr static bool is_invocable = concepts::InvocableTo<F const&, R, meta::Like<Self, T>, Args...>;

        template<typename T>
        struct Invocable {
            template<typename F>
            using Invoke = Constexpr<is_invocable<T, F>>;
        };

        template<typename T>
        requires(concepts::TagInvocableTo<Tag, R, meta::Like<Self, T>, Args...> ||
                 (concepts::InvocableTo<Tags const&, R, meta::Like<Self, T>, Args...> || ...))
        constexpr auto operator()(T&& self, Args... args) const -> R {
            if constexpr (concepts::TagInvocableTo<Tag, R, meta::Like<Self, T>, Args...>) {
                auto const tag = Tag {};
                return function::tag_invoke(tag, util::forward_like<Self>(self), util::forward<Args>(args)...);
            } else {
                using Matching = meta::Filter<meta::List<Tags...>, Invocable<T>>;
                using Choice = meta::Front<Matching>;
                auto const tag = Choice {};
                return function::invoke_r<R>(tag, util::forward_like<Self>(self), util::forward<Args>(args)...);
            }
        }
    };
}

template<typename Self, concepts::LanguageFunction Sig, typename... Tags>
using Dispatcher = detail::DispatcherImpl<Self, Sig, Tags...>;
}

namespace di {
using any::Dispatcher;
}
