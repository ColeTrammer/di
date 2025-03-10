#pragma once

#include "di/function/tag_invoke.h"
#include "di/io/interface/writer.h"
#include "di/io/size_writer.h"
#include "di/meta/language.h"
#include "di/reflect/reflect.h"
#include "di/types/in_place_type.h"
#include "di/util/declval.h"

namespace di::concepts {
template<typename T>
concept Serializer = requires(T& serializer) {
    typename meta::RemoveCVRef<T>::SerializationFormat;
    { serializer.writer() } -> Impl<io::Writer>;
    util::as_const(serializer).writer();
    util::move(serializer).writer();
};
}

namespace di::meta {
template<concepts::Serializer S>
using SerializationFormat = typename meta::RemoveCVRef<S>::SerializationFormat;
}

namespace di::serialization {
namespace detail {
    struct SerializerFunction {
        template<typename Format, concepts::Impl<io::Writer> Writer, typename... Args>
        requires(concepts::TagInvocable<SerializerFunction, Format, Writer, Args...> ||
                 requires { Format::serializer(util::declval<Writer>(), util::declval<Args>()...); })
        constexpr auto operator()(Format, Writer&& writer, Args&&... args) const -> concepts::Serializer auto {
            if constexpr (concepts::TagInvocable<SerializerFunction, Format, Writer, Args...>) {
                return function::tag_invoke(*this, Format {}, util::forward<Writer>(writer),
                                            util::forward<Args>(args)...);
            } else {
                return Format::serializer(util::forward<Writer>(writer), util::forward<Args>(args)...);
            }
        }
    };
}

constexpr inline auto serializer = detail::SerializerFunction {};
}

namespace di::concepts {
template<typename T, typename Writer = any::AnyRef<io::Writer>, typename... Args>
concept SerializationFormat = requires(T format, Writer&& writer, Args&&... args) {
    serialization::serializer(format, util::forward<Writer>(writer), util::forward<Args>(args)...);
};
}

namespace di::meta {
template<typename T, typename Writer = any::AnyRef<io::Writer>, typename... Args>
requires(concepts::SerializationFormat<T, Writer, Args...>)
using Serializer =
    decltype(serialization::serializer(util::declval<T>(), util::declval<Writer>(), util::declval<Args>()...));

template<typename S>
using SerializeResult = meta::WriterResult<void, decltype(util::declval<S>().writer())>;
}

namespace di::serialization {
namespace detail {
    struct SerializeMetadataFunction {
        template<typename T, typename S, typename U = meta::RemoveCVRef<T>,
                 concepts::SerializationFormat V = meta::RemoveCVRef<S>>
        requires(concepts::TagInvocable<SerializeMetadataFunction, InPlaceType<U>, InPlaceType<V>> ||
                 concepts::TagInvocable<SerializeMetadataFunction, InPlaceType<U>> || concepts::Reflectable<T>)
        constexpr auto operator()(InPlaceType<T>, InPlaceType<S>) const -> concepts::ReflectionValue auto {
            if constexpr (concepts::TagInvocable<SerializeMetadataFunction, InPlaceType<U>, InPlaceType<V>>) {
                return function::tag_invoke(*this, in_place_type<U>, in_place_type<V>);
            } else if constexpr (concepts::TagInvocable<SerializeMetadataFunction, InPlaceType<U>>) {
                return function::tag_invoke(*this, in_place_type<U>);
            } else {
                return reflection::reflect(in_place_type<U>);
            }
        }
    };
}

constexpr inline auto serialize_metadata = detail::SerializeMetadataFunction {};
}

namespace di::meta {
template<concepts::SerializationFormat S, typename T>
using SerializeMetadata = decltype(serialization::serialize_metadata(in_place_type<T>, in_place_type<S>));
}

namespace di::serialization {
namespace detail {
    struct SerializeFunction {
        template<concepts::Serializer S, typename T, typename F = meta::SerializationFormat<S>>
        requires(concepts::TagInvocable<SerializeFunction, F, S&, T&> ||
                 requires(S& serializer, T& value) { serializer.serialize(value); })
        constexpr auto operator()(S&& serializer, T&& value) const -> meta::SerializeResult<S> {
            if constexpr (concepts::TagInvocable<SerializeFunction, F, S&, T&>) {
                return function::tag_invoke(*this, F(), serializer, value);
            } else {
                return serializer.serialize(value);
            }
        }

        template<concepts::Serializer S, typename T, typename F = meta::SerializationFormat<S>,
                 typename M = meta::SerializeMetadata<F, T>>
        requires(!concepts::TagInvocable<SerializeFunction, F, S&, T&> &&
                 !requires(S& serializer, T& value) { serializer.serialize(value); } &&
                 (concepts::TagInvocable<SerializeFunction, S&, T&, M> ||
                  requires(S& serializer, T& value) { serializer.serialize(value, M()); }))
        constexpr auto operator()(S&& serializer, T&& value) const -> meta::SerializeResult<S> {
            if constexpr (concepts::TagInvocable<SerializeFunction, S&, T&, M>) {
                return function::tag_invoke(*this, serializer, value, M());
            } else {
                return serializer.serialize(value, M());
            }
        }

        template<typename Format, concepts::Impl<io::Writer> Writer, typename T, typename... Args>
        requires(concepts::SerializationFormat<Format, util::ReferenceWrapper<meta::RemoveReference<Writer>>, Args...>)
        constexpr auto operator()(Format format, Writer&& writer, T&& value, Args&&... args) const
        requires(requires {
            (*this)(serialization::serializer(format, util::ref(writer), util::forward<Args>(args)...), value);
        })
        {
            return (*this)(serialization::serializer(format, util::ref(writer), util::forward<Args>(args)...), value);
        }
    };
}

constexpr inline auto serialize = detail::SerializeFunction {};

namespace detail {
    struct SerializeSizeFunction {
        template<typename Format, typename T, typename... Args>
        constexpr auto operator()(Format format, T&& value, Args&&... args) const
        requires(concepts::TagInvocable<SerializeSizeFunction, Format, T, Args...> ||
                 requires {
                     serialization::serialize(
                         serialization::serializer(format, di::declval<SizeWriter>(), util::forward<Args>(args)...),
                         value);
                 })
        {
            if constexpr (concepts::TagInvocable<SerializeSizeFunction, Format, T, Args...>) {
                return function::tag_invoke(*this, format, value, args...);
            } else {
                auto writer = SizeWriter {};
                auto serializer = serialization::serializer(format, util::ref(writer), util::forward<Args>(args)...);
                (void) serialization::serialize(serializer, value);
                return writer.written();
            }
        }
    };
}

constexpr inline auto serialize_size = detail::SerializeSizeFunction {};

namespace detail {
    struct SerializableFunction {
        template<concepts::Serializer S, typename T, typename U = meta::RemoveCVRef<T>>
        constexpr auto operator()(InPlaceType<S>, InPlaceType<T>) const -> bool {
            if constexpr (concepts::TagInvocable<SerializableFunction, meta::SerializationFormat<S>, InPlaceType<U>>) {
                return function::tag_invoke(*this, meta::SerializationFormat<S>(), in_place_type<U>);
            } else {
                return requires { serialize(util::declval<S>(), util::declval<T>()); };
            }
        }
    };
}

constexpr inline auto serializable = detail::SerializableFunction {};
}

namespace di::concepts {
template<typename T, typename S>
concept Serializable =
    concepts::Serializer<S> && requires { serialization::serializable(in_place_type<S>, in_place_type<T>); };
}

namespace di {
using concepts::Serializable;
using concepts::SerializationFormat;
using concepts::Serializer;

using meta::SerializeMetadata;
using meta::SerializeResult;

using serialization::serialize;
using serialization::serialize_size;
using serialization::serializer;
}
