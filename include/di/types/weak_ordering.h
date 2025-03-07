#pragma once

#include "di/types/compare_outcome.h"
#include "di/types/partial_ordering.h"

#ifndef DI_NO_USE_STD
#include <compare>
#else
namespace std {
class weak_ordering {
public:
    static weak_ordering const less;
    static weak_ordering const equivalent;
    static weak_ordering const greater;

    constexpr friend auto operator==(weak_ordering v, weak_ordering w) -> bool { return v.m_value == w.m_value; }
    constexpr friend auto operator==(weak_ordering v, int) -> bool { return v.m_value == 0; }

    constexpr friend auto operator<(weak_ordering v, int) -> bool { return v.m_value < 0; }
    constexpr friend auto operator<(int, weak_ordering v) -> bool { return v.m_value > 0; }

    constexpr friend auto operator<=(weak_ordering v, int) -> bool { return v == 0 || v < 0; }
    constexpr friend auto operator<=(int, weak_ordering v) -> bool { return v == 0 || 0 < v; }

    constexpr friend auto operator>(weak_ordering v, int) -> bool { return 0 < v; }
    constexpr friend auto operator>(int, weak_ordering v) -> bool { return v < 0; }

    constexpr friend auto operator>=(weak_ordering v, int) -> bool { return 0 <= v; }
    constexpr friend auto operator>=(int, weak_ordering v) -> bool { return v <= 0; }

    constexpr friend auto operator<=>(weak_ordering v, int) -> weak_ordering { return v; }
    constexpr friend auto operator<=>(int, weak_ordering v) -> weak_ordering { return weak_ordering(char(-v.m_value)); }

    constexpr operator partial_ordering() const { return partial_ordering(m_value); }

private:
    friend class strong_ordering;

    explicit constexpr weak_ordering(char value) : m_value(value) {}
    explicit constexpr weak_ordering(di::types::detail::CompareOutcome value) : m_value(static_cast<char>(value)) {}

    char m_value;
};

constexpr inline weak_ordering weak_ordering::less(di::types::detail::CompareOutcome::Less);
constexpr inline weak_ordering weak_ordering::equivalent(di::types::detail::CompareOutcome::Equal);
constexpr inline weak_ordering weak_ordering::greater(di::types::detail::CompareOutcome::Greater);
}
#endif

namespace di::types {
using std::weak_ordering;
}

namespace di {
using types::weak_ordering;
}
