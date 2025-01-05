#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Enumerate.h
// 2025.01.05 김승범   - <ranges> 에서 23 의 내용인 Enumerate 만 추출하여 사용하도록 추가함. 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <vcruntime.h>
#include <ranges>

#include <tuple>
#include <type_traits>
#include <utility>
#include <iterator>
#include <concepts>
// C++23 을 사용하는 경우 std::ranges::enumerate_view 를 사용할 것
#if _HAS_CXX20 && !_HAS_CXX23 

template <typename Range>
concept RangeWithMovableReferences =
std::ranges::input_range<Range> &&
std::move_constructible<std::ranges::range_reference_t<Range>> &&
std::move_constructible<std::ranges::range_rvalue_reference_t<Range>>;

template <std::ranges::view V>
    requires RangeWithMovableReferences<V>
class EnumerateView : public std::ranges::view_interface<EnumerateView<V>> {
private:
    V mBaseRange;

    template <bool IsConst>
    class Iterator {
    private:
        using Base = std::conditional_t<IsConst, const V, V>;
        using BaseIterator = std::ranges::iterator_t<Base>;
        using ReferenceType = std::tuple<std::ranges::range_difference_t<Base>, std::ranges::range_reference_t<Base>>;

        BaseIterator mCurrent;
        std::ranges::range_difference_t<Base> mPosition = 0;

        friend class EnumerateView;

        constexpr explicit Iterator(BaseIterator current, std::ranges::range_difference_t<Base> position) noexcept
            : mCurrent(std::move(current)), mPosition(position) {}

    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ranges::range_difference_t<Base>;
        using value_type = std::tuple<difference_type, std::ranges::range_value_t<Base>>;

        Iterator() = default;

        constexpr Iterator(const Iterator<!IsConst>& other) noexcept
            requires IsConst&& std::convertible_to<std::ranges::iterator_t<V>, BaseIterator>
        : mCurrent(other.mCurrent), mPosition(other.mPosition) {}

        constexpr ReferenceType operator*() const noexcept {
            return ReferenceType{ mPosition, *mCurrent };
        }

        constexpr Iterator& operator++() noexcept {
            ++mCurrent;
            ++mPosition;
            return *this;
        }

        constexpr void operator++(int) noexcept {
            ++(*this);
        }

        friend constexpr bool operator==(const Iterator& lhs, const Iterator& rhs) noexcept {
            return lhs.mCurrent == rhs.mCurrent;
        }

        friend constexpr auto operator<=>(const Iterator& lhs, const Iterator& rhs) noexcept {
            return lhs.mPosition <=> rhs.mPosition;
        }
    };

    template <bool IsConst>
    class Sentinel {
    private:
        using Base = std::conditional_t<IsConst, const V, V>;
        using BaseSentinel = std::ranges::sentinel_t<Base>;

        BaseSentinel mEnd;

        friend class EnumerateView;

        constexpr explicit Sentinel(BaseSentinel end) noexcept
            : mEnd(std::move(end)) {}

		constexpr bool Equal(const Iterator<IsConst>& iter) const noexcept {
			return iter.mCurrent == mEnd;
		}

    public:
        Sentinel() = default;

        constexpr Sentinel(const Sentinel<!IsConst>& other) noexcept
            requires IsConst&& std::convertible_to<std::ranges::sentinel_t<V>, BaseSentinel>
        : mEnd(std::move(other.mEnd)) {}

        template <bool OtherConst>
        friend constexpr bool operator==(const Iterator<OtherConst>& iter, const Sentinel& sent) noexcept {
            return sent.Equal(iter);
        }
    };

public:
    EnumerateView() = default;

    constexpr explicit EnumerateView(V base) noexcept(std::is_nothrow_move_constructible_v<V>)
        : mBaseRange(std::move(base)) {}

    constexpr auto begin() noexcept {
        return Iterator<false>{ std::ranges::begin(mBaseRange), 0 };
    }

    constexpr auto begin() const noexcept
        requires RangeWithMovableReferences<const V> {
        return Iterator<true>{ std::ranges::begin(mBaseRange), 0 };
    }

    constexpr auto end() noexcept {
        return Sentinel<false>{ std::ranges::end(mBaseRange) };
    }

    constexpr auto end() const noexcept
        requires RangeWithMovableReferences<const V> {
        return Sentinel<true>{ std::ranges::end(mBaseRange) };
    }

    constexpr auto Size() noexcept
        requires std::ranges::sized_range<V> {
        return std::ranges::size(mBaseRange);
    }

    constexpr auto Size() const noexcept
        requires std::ranges::sized_range<const V> {
        return std::ranges::size(mBaseRange);
    }

    constexpr V Base() const& noexcept
        requires std::copy_constructible<V> {
        return mBaseRange;
    }

    constexpr V Base() && noexcept {
        return std::move(mBaseRange);
    }
};

template <typename Range>
EnumerateView(Range&&) -> EnumerateView<std::views::all_t<Range>>;

// Helper function
template <typename Range>
constexpr auto Enumerate(Range&& range) {
    return EnumerateView<std::views::all_t<Range>>(std::forward<Range>(range));
}
    


#endif 