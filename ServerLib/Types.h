#pragma once 

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Types.h
// 2025 - 01 - 14 (설명 추가 날짜)
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

using SessionIdType = BYTE;
using PacketSizeType = BYTE;

using ExtraInfo = std::variant<void*, SOCKET, unsigned long long, HANDLE>;

// --------------------------------- Template ---------------------------------
template <size_t size>
using NetworkBuf = std::array<char, size>;

template <size_t size>
using NetworkBufIter = NetworkBuf<size>::iterator;

struct Key {
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