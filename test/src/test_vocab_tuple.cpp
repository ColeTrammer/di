#include "di/meta/constexpr.h"
#include "di/test/prelude.h"
#include "di/vocab/tuple/prelude.h"

namespace vocab_tuple {
class X : public di::util::AddMemberGet<X> {
public:
private:
    constexpr friend auto tag_invoke(di::Tag<di::vocab::tuple_size>, di::InPlaceType<X>) -> size_t { return 1ZU; }

    constexpr friend auto tag_invoke(di::Tag<di::vocab::tuple_element>, di::InPlaceType<X>, di::Constexpr<0ZU>)
        -> di::InPlaceType<int>;

    template<di::concepts::DecaySameAs<X> Self>
    constexpr friend auto tag_invoke(di::Tag<di::util::get_in_place>, di::Constexpr<0ZU>, Self&& self)
        -> di::meta::Like<Self, int> {
        return di::util::forward_like<Self>(self.x);
    }

    int x;
};

static_assert(di::concepts::detail::HasTupleElement<X, 0>);
static_assert(di::concepts::detail::HasTupleGet<X, 0>);

static_assert(di::concepts::TupleLike<X>);
static_assert(di::concepts::detail::CanStructuredBind<X>);

constexpr static void enable_structed_bindings() {
    auto x = X {};
    auto [y] = x;

    auto const z = X {};
    auto [zz] = z;

    static_assert(di::concepts::SameAs<decltype(zz), int>);

    (void) x.get<0>();

    (void) y;
    (void) zz;
}

struct XX {
    int x;
    int y;
    long z;
};

struct XXX {
    int x;
    long y;
    int z;
};

struct XXXX {
    long x;
    int y;
    int z;
};

struct XXXXX {
    int x;
    long y;
    int z;
    long xx;
    int yy;
    long zz;
};

static_assert(sizeof(XX) == sizeof(di::Tuple<XX>));
static_assert(sizeof(XX) == sizeof(di::Tuple<int, int, long>));
static_assert(sizeof(XXX) == sizeof(di::Tuple<int, long, int>));
static_assert(sizeof(XXXX) == sizeof(di::Tuple<long, int, int>));
static_assert(sizeof(XXXXX) == sizeof(di::Tuple<int, long, int, long, int, long>));

constexpr static void basic() {
    static_assert(di::concepts::TupleLike<di::Tuple<int, int, int>>);

    auto x = di::Tuple<int, int, int> {};

    auto e = di::get<2>(x);
    ASSERT_EQ(e, 0);

    auto f = x.get<0>();
    ASSERT_EQ(f, 0);

    static_assert(
        di::concepts::detail::CanStructuredBindHelper<di::Tuple<int, int, int>, di::meta::ListV<0ZU, 1ZU, 2ZU>>::value);
    static_assert(di::concepts::detail::CanStructuredBind<di::Tuple<int, int, int>>);

    auto [a, b, c] = x;
    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 0);
    ASSERT_EQ(c, 0);

    using Z = di::vocab::TupleImpl<di::meta::IndexSequenceFor<int, int, int>, int, int, int>;
    static_assert(di::concepts::ConstructibleFrom<Z, di::vocab::ConstructTupleImplFromTuplelike,
                                                  di::Tuple<short, short, int> const&>);
    static_assert(di::concepts::ConstructibleFrom<di::Tuple<int, int, int>, di::Tuple<short, short, int> const&>);
    static_assert(di::concepts::ConstructibleFrom<di::Tuple<int, int, int>, di::Tuple<short, short, int>&>);
    static_assert(di::concepts::ConstructibleFrom<di::Tuple<int, int, int>, di::Tuple<short, short, int> const&&>);
    static_assert(di::concepts::ConstructibleFrom<di::Tuple<int, int, int>, di::Tuple<short, short, int>&&>);

    auto y = di::Tuple<int, int, int>(di::Tuple<short, short, int>(1, 2, 3));
    auto [i, j, k] = y;
    ASSERT_EQ(i, 1);
    ASSERT_EQ(j, 2);
    ASSERT_EQ(k, 3);

    auto const z = di::make_tuple(9, 9);
    auto const& [n, m] = z;
    ASSERT_EQ(n, 9);
    ASSERT_EQ(m, 9);
    static_assert(di::concepts::SameAs<decltype((n)), int const&>);
    static_assert(di::concepts::SameAs<decltype((m)), int const&>);

    static_assert(di::concepts::SameAs<std::tuple_element<0, di::Tuple<int, int> const>::type, int const>);
}

constexpr static void assignment() {
    int a = 2;
    int b = 3;
    int c = 4;
    auto const x = di::tie(a, b, c);

    int d = 5;
    int e = 6;
    int f = 7;
    x = di::tie(d, e, f);

    ASSERT_EQ(a, 5);
    ASSERT_EQ(e, 6);
    ASSERT_EQ(f, 7);

    auto y = di::make_tuple(6L, 5L, 4L);
    y = di::make_tuple(4L, 3L, 2L);
    ASSERT_EQ(di::get<0>(y), 4);
    ASSERT_EQ(di::get<1>(y), 3);
    ASSERT_EQ(di::get<2>(y), 2);

    y = di::make_tuple(3, 2, 1);
    ASSERT_EQ(di::get<0>(y), 3);
    ASSERT_EQ(di::get<1>(y), 2);
    ASSERT_EQ(di::get<2>(y), 1);
}

constexpr static void tuple_transform() {
    auto x = di::make_tuple(3, 2, 1);
    auto y = di::tuple_transform(di::identity, x);
    ASSERT_EQ(di::get<0>(x), di::get<0>(y));
}

constexpr static void tuple_for_each() {
    auto x = di::make_tuple(3, 2, 1);
    auto s = 0;
    di::tuple_for_each(
        [&](auto i) {
            s += i;
        },
        x);
    ASSERT_EQ(s, 6);
}

constexpr static void tuple_equal() {
    auto a = di::make_tuple(5, 3, 2);
    auto b = di::make_tuple(5, 3, 2);
    ASSERT_EQ(a, b);

    auto c = di::make_tuple(5L, 3L, 2L);
    ASSERT_EQ(a, c);

    ASSERT_EQ(a <=> b, di::strong_ordering::equal);
    auto d = di::make_tuple(5L, 3L, 3L);
    ASSERT_EQ(a <=> d, di::strong_ordering::less);

    static_assert(!di::concepts::EqualityComparableWith<di::Tuple<int>, di::Tuple<int, int>>);
    static_assert(!di::concepts::EqualityComparableWith<di::Tuple<int, di::Void>, di::Tuple<int, int>>);
}

constexpr static void tuple_cat() {
    auto a = di::make_tuple(1, 2);
    int x = 5;
    int y = 6;
    int z = 7;
    auto b = di::tie(x, y);
    auto c = di::forward_as_tuple<int>(di::move(z));

    auto r = di::tuple_cat(a, b, di::move(c));
    static_assert(di::SameAs<decltype(r), di::Tuple<int, int, int&, int&, int&&>>);
    ASSERT_EQ(r, di::make_tuple(1, 2, 5, 6, 7));
}

TESTC(vocab_tuple, enable_structed_bindings)
TESTC(vocab_tuple, basic)
TESTC(vocab_tuple, assignment)
TESTC(vocab_tuple, tuple_transform)
TESTC(vocab_tuple, tuple_for_each)
TESTC(vocab_tuple, tuple_equal)
TESTC(vocab_tuple, tuple_cat)
}
