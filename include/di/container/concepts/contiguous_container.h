#pragma once

#include "di/container/concepts/contiguous_iterator.h"
#include "di/container/concepts/random_access_container.h"
#include "di/container/interface/data.h"
#include "di/container/meta/container_reference.h"
#include "di/meta/core.h"
#include "di/meta/language.h"

namespace di::concepts {
template<typename T>
concept ContiguousContainer =
    RandomAccessContainer<T> && ContiguousIterator<meta::ContainerIterator<T>> && requires(T& value) {
        { container::data(value) } -> SameAs<meta::AddPointer<meta::ContainerReference<T>>>;
    };
}
