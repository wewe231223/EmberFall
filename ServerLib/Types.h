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