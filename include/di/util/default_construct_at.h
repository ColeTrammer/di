#pragma once

#include "di/util/construct_at.h"
#include "di/util/std_new.h"
#include "di/util/voidify.h"

namespace di::util {
namespace detail {
    struct DefaultConstructAtFunction {
        template<typename T>
        requires(requires(void* pointer) { ::new (pointer) T; })
        constexpr auto operator()(T* location) const -> T* {
            // NOTE: this is not actually the same behavior, as the
            //       expression shown below leaves trivial types
            //       uninitialized. However, placement new is not
            //       usable in constexpr context, so we cannot
            //       call it here.
            if consteval {
                return std::construct_at(location);
            } else {
                return ::new (util::voidify(location)) T;
            }
        }
    };
}

constexpr inline auto default_construct_at = detail::DefaultConstructAtFunction {};
}

namespace di {
using util::default_construct_at;
}
