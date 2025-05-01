#pragma once 

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Types.h
// 2025 - 01 - 14 (설명 추가 날짜)
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// m/s^2
#define M_PER_SEC2
// m/s
#define M_PER_SEC
// cm/s
#define CM_PER_SEC
// km/s
#define KM_PER_SEC
// km/h
#define KM_PER_HOUR

using SessionIdType = BYTE;
using PacketSizeType = BYTE;
using NetworkObjectIdType = unsigned short;

using ExtraInfo = std::variant<void*, SOCKET, unsigned long long, HANDLE>;

enum class ObjectTag : uint8_t {
    ENV,
    BOSSPLAYER,
    PLAYER,
    MONSTER,
    CORRUPTED_GEM,
    ITEM,
    TRIGGER,
    ARROW,
    NONE,
};

// --------------------------------- Template ---------------------------------
template <size_t size>
using NetworkBuf = std::array<char, size>;

template <size_t size>
using NetworkBufIter = NetworkBuf<size>::iterator;

struct Key {
    inline static constexpr bool DOWN = true;
    inline static constexpr bool UP = false;

    BYTE key;
    bool state;
};

template <size_t size>
struct NetworkBufRange {
    NetworkBufIter<size> start;
    NetworkBufIter<size> end;

    size_t Size() {
        return static_cast<size_t>(std::distance(start, end));
    }
};

namespace TypeList {
    struct MyContType { };
    struct ContTypeList { };

    template <typename... Types>
    struct TypeList { using Type = ContTypeList; };

    template <typename T, typename U>
    struct TypeList<T, U> {
        using Type = ContTypeList;
        using Head = T;
        using Tail = U;
    };

    template <typename T, typename... Types>
    struct TypeList<T, Types...> {
        using Type = ContTypeList;
        using Head = T;
        using Tail = TypeList<T, Types...>::Tail;
    };
}

template <typename T, typename... Types>
inline constexpr bool IsAnyOf = (std::is_same_v<T, Types> || ...);

template <typename T, typename... Types>
inline constexpr bool IsAllOf = (std::is_same_v<T, Types> and ...);

template <typename Base, typename... Types>
inline constexpr bool IsDerivedFrom = (std::derived_from<Types, Base> and ...);

struct Short2 {
    int16_t x;
    int16_t y;

    Short2() : x{ }, y{ } { }
    Short2(int16_t x, int16_t y) : x{ x }, y{ y } {}

    Short2(const Short2& other) = default;
    Short2(Short2&& other) noexcept = default;
    Short2& operator=(const Short2& other) = default;
    Short2& operator=(Short2&& other) noexcept = default;

    bool operator==(Short2 rhs) const {
        return x == rhs.x and y == rhs.y;
    }

    Short2 operator-(Short2 rhs) {
        return Short2{ static_cast<int16_t>(x - rhs.x), static_cast<int16_t>(y - rhs.y) };
    }
};