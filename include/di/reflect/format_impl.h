#pragma once

#include "di/format/formatter.h"
#include "di/format/make_format_args.h"
#include "di/format/vpresent_encoded_context.h"
#include "di/reflect/enum_to_string.h"
#include "di/reflect/reflect.h"
#include "di/types/in_place_type.h"

namespace di::format {
template<concepts::ReflectableToFields T, concepts::Encoding Enc>
constexpr auto tag_invoke(types::Tag<formatter_in_place>, InPlaceType<T>, FormatParseContext<Enc>&) {
    auto do_output = [=](concepts::FormatContext auto& context, T const& value) -> Result<void> {
        auto reflection_value = reflection::reflect(value);
        for (auto ch : reflection_value.name) {
            context.output(ch);
        }
        context.output(' ');
        context.output('{');
        context.output(' ');
        bool first = true;
        (void) vocab::tuple_for_each(
            [&](auto field) {
                if (!first) {
                    context.output(',');
                    context.output(' ');
                }
                first = false;

                for (auto ch : field.name) {
                    context.output(ch);
                }
                context.output(':');
                context.output(' ');
                (void) vpresent_encoded_context<meta::Encoding<decltype(context)>>(
                    u8"{}"_sv, format::make_format_args<decltype(context)>(field.get(value)), context, true);
            },
            reflection_value);

        context.output(' ');
        context.output('}');
        return {};
    };
    return Result<decltype(do_output)>(util::move(do_output));
}

template<concepts::ReflectableToEnumerators T, concepts::Encoding Enc>
constexpr auto tag_invoke(types::Tag<formatter_in_place>, InPlaceType<T>, FormatParseContext<Enc>& parse_context,
                          bool debug) {
    using R = decltype(reflection::enum_to_string(di::declval<T>()));
    return format::formatter<R, Enc>(parse_context, debug) % [](concepts::CopyConstructible auto formatter) {
        return [=](concepts::FormatContext auto& context, T value) {
            return formatter(context, reflection::enum_to_string(value));
        };
    };
}
}
