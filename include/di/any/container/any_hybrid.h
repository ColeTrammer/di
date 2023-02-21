#pragma once

#include <di/any/container/any.h>

namespace di::any {
template<concepts::Interface Interface, size_t inline_size = 2 * sizeof(void*), size_t inline_align = alignof(void*)>
using AnyHybrid = Any<Interface, HybridStorage<inline_size, inline_align>>;
}
