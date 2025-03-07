#pragma once

#include "di/function/invoke.h"
#include "di/function/pipeable.h"
#include "di/meta/util.h"
#include "di/types/in_place.h"
#include "di/util/forward.h"
#include "di/util/move.h"

namespace di::function {
namespace detail {
    template<concepts::DecayConstructible F>
    struct FlipFunction : public pipeline::EnablePipeline {
    public:
        template<typename Fn>
        constexpr FlipFunction(types::InPlace, Fn&& function) : m_function(util::forward<Fn>(function)) {}

        constexpr FlipFunction(FlipFunction const&) = default;
        constexpr FlipFunction(FlipFunction&&) = default;

        constexpr auto operator=(FlipFunction const&) -> FlipFunction& = delete;
        constexpr auto operator=(FlipFunction&&) -> FlipFunction& = delete;

        template<typename T, typename U>
        requires(concepts::Invocable<F&, T, U>)
        constexpr auto operator()(U&& a, T&& b) & -> decltype(auto) {
            return function::invoke(m_function, util::forward<U>(b), util::forward<T>(a));
        }

        template<typename T, typename U>
        requires(concepts::Invocable<F const&, T, U>)
        constexpr auto operator()(U&& a, T&& b) const& -> decltype(auto) {
            return function::invoke(m_function, util::forward<U>(b), util::forward<T>(a));
        }

        template<typename T, typename U>
        requires(concepts::Invocable<F &&, T, U>)
        constexpr auto operator()(U&& a, T&& b) && -> decltype(auto) {
            return function::invoke(util::move(m_function), util::forward<U>(b), util::forward<T>(a));
        }

        template<typename T, typename U>
        requires(concepts::Invocable<F const &&, T, U>)
        constexpr auto operator()(U&& a, T&& b) const&& -> decltype(auto) {
            return function::invoke(util::move(m_function), util::forward<U>(b), util::forward<T>(a));
        }

    private:
        F m_function;
    };
}

template<concepts::DecayConstructible F>
constexpr auto flip(F&& function) {
    return detail::FlipFunction<meta::Decay<F>>(types::in_place, util::forward<F>(function));
}
}

namespace di {
using function::flip;
}
