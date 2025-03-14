#pragma once

#include "di/container/concepts/contiguous_iterator.h"
#include "di/container/concepts/sized_sentinel_for.h"
#include "di/container/iterator/iterator_base.h"
#include "di/container/meta/iterator_const_reference.h"
#include "di/container/meta/iterator_reference.h"
#include "di/container/meta/iterator_ssize_type.h"
#include "di/container/meta/iterator_value.h"
#include "di/meta/common.h"
#include "di/meta/compare.h"
#include "di/util/move.h"
#include "di/util/to_address.h"

namespace di::container {
template<typename Iter>
class ConstIteratorImpl
    : public IteratorBase<ConstIteratorImpl<Iter>, meta::IteratorCategory<Iter>, meta::IteratorValue<Iter>,
                          meta::IteratorSSizeType<Iter>> {
private:
    using Self = ConstIteratorImpl;
    using SSizeType = meta::IteratorSSizeType<Iter>;
    using Value = meta::IteratorValue<Iter>;

public:
    ConstIteratorImpl()
    requires(concepts::DefaultInitializable<Iter>)
    = default;
    constexpr ConstIteratorImpl(Iter iter) : m_base(util::move(iter)) {}

    template<concepts::ConvertibleTo<Iter> Jt>
    constexpr ConstIteratorImpl(ConstIteratorImpl<Jt> other) : m_base(util::move(other.base())) {}

    template<concepts::ConvertibleTo<Iter> T>
    constexpr ConstIteratorImpl(T&& other) : m_base(util::forward<T>(other)) {}

    constexpr auto base() const& -> Iter const& { return m_base; }
    constexpr auto base() && -> Iter { return util::move(m_base); }

    constexpr auto operator*() const -> meta::IteratorConstReference<Iter> { return *m_base; }

    constexpr auto operator->() const
        -> Value const* requires(concepts::ContiguousIterator<Iter>) { return util::to_address(m_base); }

    constexpr void advance_one() {
        ++m_base;
    }
    constexpr void back_one()
    requires(concepts::BidirectionalIterator<Iter>)
    {
        --m_base;
    }

    constexpr void advance_n(SSizeType n)
    requires(concepts::RandomAccessIterator<Iter>)
    {
        m_base += n;
    }

private:
    constexpr friend auto operator==(Self const& a, Self const& b) -> bool
    requires(concepts::SentinelFor<Iter, Iter>)
    {
        return a.base() == b.base();
    }

    template<typename Sent>
    requires(!concepts::SameAs<Self, Sent> && concepts::SentinelFor<Sent, Iter>)
    constexpr friend auto operator==(Self const& a, Sent const& b) -> bool {
        return a.base() == b;
    }

    constexpr friend auto operator<=>(Self const& a, Self const& b)
    requires(concepts::RandomAccessIterator<Iter>)
    {
        return a.base() <=> b.base();
    }

    template<typename Other>
    requires(!concepts::SameAs<Self, Other> && concepts::RandomAccessIterator<Iter> &&
             concepts::TotallyOrderedWith<Iter, Other>)
    constexpr friend auto operator<=>(Self const& a, Other const& b) {
        return a.base() <=> b;
    }

    constexpr friend auto operator-(Self const& a, Self const& b) -> SSizeType
    requires(concepts::RandomAccessIterator<Iter>)
    {
        return a.base() - b.base();
    }

    template<typename Sent>
    requires(!concepts::SameAs<Sent, Iter> && concepts::SizedSentinelFor<Sent, Iter>)
    constexpr auto operator-(Sent const& b) -> SSizeType {
        return this->base() - b;
    }

    template<typename Sent>
    requires(!concepts::SameAs<Sent, Self> && concepts::SizedSentinelFor<Sent, Iter>)
    constexpr friend auto operator-(Sent const& a, Self const& b) -> SSizeType {
        return a - b.base();
    }

    Iter m_base;
};
}

template<typename T, di::concepts::CommonWith<T> U>
struct di::meta::CustomCommonType<di::container::ConstIteratorImpl<T>, U> {
    using Type = di::container::ConstIteratorImpl<di::meta::CommonType<T, U>>;
};

template<typename T, di::concepts::CommonWith<T> U>
struct di::meta::CustomCommonType<U, di::container::ConstIteratorImpl<T>> {
    using Type = di::container::ConstIteratorImpl<di::meta::CommonType<T, U>>;
};

template<typename T, di::concepts::CommonWith<T> U>
struct di::meta::CustomCommonType<di::container::ConstIteratorImpl<T>, di::container::ConstIteratorImpl<U>> {
    using Type = di::container::ConstIteratorImpl<di::meta::CommonType<T, U>>;
};
