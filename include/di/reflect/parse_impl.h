#pragma once

#include "di/container/view/split.h"
#include "di/meta/core.h"
#include "di/parser/basic/match_one_or_more.h"
#include "di/parser/make_error.h"
#include "di/parser/prelude.h"
#include "di/reflect/reflect.h"
#include "di/util/bitwise_enum.h"
#include "di/vocab/expected/unexpected.h"
#include "di/vocab/tuple/tuple_for_each.h"

namespace di::parser::detail {
template<concepts::SameAs<types::Tag<parser::create_parser_in_place>> Tag = types::Tag<parser::create_parser_in_place>,
         concepts::ReflectableToEnumerators T>
constexpr auto tag_invoke(Tag, InPlaceType<T>) {
    auto valid_code_point = [](c32 code_point) {
        constexpr auto valid_char_table = [] {
            auto table = vocab::Array<bool, 256> {};
            vocab::tuple_for_each(
                [&](auto enumerator) {
                    for (auto ch : enumerator.name) {
                        table[ch] = true;
                    }
                    if constexpr (concepts::BitwiseEnum<T>) {
                        table['|'] = true;
                    }
                },
                reflection::reflect(in_place_type<T>));
            return table;
        }();

        return code_point < 256 && valid_char_table[code_point];
    };

    return parser::match_one_or_more(valid_code_point) <<
               []<typename Context>(Context& context,
                                    concepts::CopyConstructible auto results) -> meta::ParserContextResult<T, Context> {
        if constexpr (!concepts::BitwiseEnum<T>) {
            auto result = vocab::Optional<T> {};
            vocab::tuple_for_each(
                [&](auto enumerator) {
                    if (container::equal(results, enumerator.name)) {
                        result = enumerator.value;
                    }
                },
                reflection::reflect(in_place_type<T>));

            if (!result) {
                return vocab::Unexpected(parser::make_error(context));
            }
            return *result;
        } else {
            auto result = T(0);
            for (auto substring : split(results, U'|')) {
                auto found = false;
                vocab::tuple_for_each(
                    [&](auto enumerator) {
                        if (container::equal(substring, enumerator.name)) {
                            result |= enumerator.value;
                            found = true;
                        }
                    },
                    reflection::reflect(in_place_type<T>));
                if (!found) {
                    return vocab::Unexpected(parser::make_error(context));
                }
            }
            return result;
        }
    };
}
}
