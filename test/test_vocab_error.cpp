#include <di/vocab/error/prelude.h>
#include <dius/test/prelude.h>

namespace vocab_error {
constexpr void basic() {
    di::GenericCode e = di::BasicError::ResultOutOfRange;
    ASSERT(!e.success());
    ASSERT_EQ(e.message(), u8"Result out of range"_sv);

    ASSERT_EQ(e, di::BasicError::ResultOutOfRange);
    ASSERT_NOT_EQ(di::BasicError::Success, e);
}

void erased() {
    di::Error e = di::BasicError::ResultOutOfRange;
    ASSERT(!e.success());
    ASSERT_EQ(e.message(), u8"Result out of range"_sv);

    ASSERT_EQ(e, di::BasicError::ResultOutOfRange);
    ASSERT_NOT_EQ(di::BasicError::Success, e);
}

TESTC(vocab_error, basic)
TEST(vocab_error, erased)
}
