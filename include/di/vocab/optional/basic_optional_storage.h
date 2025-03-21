#pragma once

#include "di/meta/core.h"
#include "di/meta/operations.h"
#include "di/meta/util.h"
#include "di/types/byte.h"
#include "di/util/construct_at.h"
#include "di/util/destroy_at.h"
#include "di/util/forward.h"
#include "di/util/forward_like.h"
#include "di/vocab/optional/get_value.h"
#include "di/vocab/optional/is_nullopt.h"
#include "di/vocab/optional/nullopt.h"
#include "di/vocab/optional/set_nullopt.h"
#include "di/vocab/optional/set_value.h"

namespace di::vocab {
template<typename T>
class BasicOptionalStorage {
public:
    constexpr explicit BasicOptionalStorage(NullOpt) {
        // Suppress uninialized variable warnings by zero-initializing the storage.
        for (size_t i = 0; i < sizeof(T); i++) {
            m_array[i] = Byte(0);
        }
    }

    BasicOptionalStorage(BasicOptionalStorage const&) = default;
    BasicOptionalStorage(BasicOptionalStorage&&) = default;

    auto operator=(BasicOptionalStorage const&) -> BasicOptionalStorage& = default;
    auto operator=(BasicOptionalStorage&&) -> BasicOptionalStorage& = default;

    ~BasicOptionalStorage() = default;

    constexpr ~BasicOptionalStorage()
    requires(!concepts::TriviallyDestructible<T>)
    {}

private:
    constexpr friend auto tag_invoke(types::Tag<is_nullopt>, BasicOptionalStorage const& self) -> bool {
        return !self.m_has_value;
    }

    template<typename Self>
    requires(concepts::SameAs<meta::Decay<Self>, BasicOptionalStorage>)
    constexpr friend auto tag_invoke(types::Tag<get_value>, Self&& self) -> decltype(auto) {
        return util::forward_like<Self>(self.m_value);
    }

    constexpr friend void tag_invoke(types::Tag<set_nullopt>, BasicOptionalStorage& self) {
        if (self.m_has_value) {
            util::destroy_at(&self.m_value);
        }
        self.m_has_value = false;
    }

    template<typename... Args>
    requires(concepts::ConstructibleFrom<T, Args...>)
    constexpr friend void tag_invoke(types::Tag<set_value>, BasicOptionalStorage& self, Args&&... args) {
        util::construct_at(&self.m_value, util::forward<Args>(args)...);
        self.m_has_value = true;
    }

    bool m_has_value { false };
    union {
        T m_value;
        Byte m_array[sizeof(T)];
    };
};
}
