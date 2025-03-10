#pragma once

#include "di/bit/endian/endian.h"
#include "di/bit/operation/byteswap.h"
#include "di/format/prelude.h"

namespace di::bit {
namespace detail {
    struct HostToLittleEndianFunction {
        template<concepts::IntegralOrEnum T>
        constexpr auto operator()(T value) const -> T {
            if constexpr (Endian::Native == Endian::Little) {
                return value;
            } else {
                return byteswap(value);
            }
        }
    };
}

constexpr inline auto host_to_little_endian = detail::HostToLittleEndianFunction {};
constexpr inline auto little_endian_to_host = detail::HostToLittleEndianFunction {};

template<concepts::IntegralOrEnum T>
class [[gnu::packed]] LittleEndian {
public:
    LittleEndian() = default;

    constexpr LittleEndian(T value) { *this = value; }

    constexpr auto operator=(T value) -> LittleEndian {
        m_value = host_to_little_endian(value);
        return *this;
    }

    constexpr operator T() const { return little_endian_to_host(m_value); }

    constexpr auto value() const -> T { return *this; }

private:
    template<concepts::Encoding Enc>
    constexpr friend auto tag_invoke(types::Tag<formatter_in_place>, InPlaceType<LittleEndian>,
                                     FormatParseContext<Enc>& parse_context, bool debug) {
        return format::formatter<T, Enc>(parse_context, debug);
    }

    T m_value { 0 };
};
}

namespace di {
using bit::LittleEndian;

using bit::host_to_little_endian;
using bit::little_endian_to_host;
}
