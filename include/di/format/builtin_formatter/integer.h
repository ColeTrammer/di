#pragma once

#include "di/format/builtin_formatter/base.h"
#include "di/format/formatter.h"
#include "di/meta/language.h"
#include "di/types/prelude.h"

namespace di::format {
template<concepts::Integer T, concepts::Encoding Enc>
requires(!concepts::OneOf<T, char, c32>)
constexpr auto tag_invoke(types::Tag<formatter_in_place>, InPlaceType<T>, FormatParseContext<Enc>& parse_context) {
    return parse<detail::IntegerFormat>(parse_context.current_format_string()) % [](detail::IntegerFormat format) {
        return [=](concepts::FormatContext auto& context, T value) -> Result<void> {
            auto width = format.width.transform(&detail::Width::value);
            return detail::present_integer_to<Enc>(context, format.fill_and_align, format.sign, format.hash_tag,
                                                   format.zero, width, format.type, false, value);
        };
    };
}

template<concepts::Encoding Enc>
constexpr auto tag_invoke(types::Tag<formatter_in_place>, InPlaceType<byte>, FormatParseContext<Enc>& parse_context) {
    return parse<detail::IntegerFormat>(parse_context.current_format_string()) % [](detail::IntegerFormat format) {
        return [=](concepts::FormatContext auto& context, byte value) -> Result<void> {
            auto width = format.width.transform(&detail::Width::value);
            return detail::present_integer_to<Enc>(context, format.fill_and_align, format.sign, format.hash_tag,
                                                   format.zero, width, format.type, false, to_integer<u8>(value));
        };
    };
}
}
