#pragma once

#include <di/bit/endian/endian.h>
#include <di/bit/operation/byteswap.h>
#include <di/format/prelude.h>

namespace di::bit {
template<concepts::Integral T>
class [[gnu::packed]] BigEndian {
public:
    BigEndian() = default;

    constexpr BigEndian(T value) { *this = value; }

    constexpr BigEndian operator=(T value) {
        if constexpr (Endian::Native == Endian::Big) {
            m_value = value;
        } else {
            m_value = byteswap(value);
        }
        return *this;
    }

    constexpr operator T() const {
        if constexpr (Endian::Native == Endian::Big) {
            return m_value;
        } else {
            return byteswap(m_value);
        }
    }

private:
    template<concepts::Encoding Enc>
    constexpr friend auto tag_invoke(types::Tag<formatter_in_place>, InPlaceType<BigEndian>,
                                     FormatParseContext<Enc>& parse_context, bool debug) {
        return format::formatter<T, Enc>(parse_context, debug);
    }

    T m_value { 0 };
};
}