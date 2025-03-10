#include "di/bit/bitset/prelude.h"
#include "di/test/prelude.h"
#include "di/util/prelude.h"

namespace bit_bitset {
constexpr static void basic() {
    auto set = di::BitSet<13> {};
    set[2] = true;
    set[10] = true;
    ASSERT_EQ(set[2], true);
    ASSERT_EQ(set[10], true);
}

constexpr static void wide() {
    auto set = di::BitSet<64> {};
    set[1] = true;
    set[63] = true;

    ASSERT_EQ(0x8000000000000002LLU, di::bit_cast<u64>(set));
}

TESTC(bit_bitset, basic)
TESTC(bit_bitset, wide)
}
