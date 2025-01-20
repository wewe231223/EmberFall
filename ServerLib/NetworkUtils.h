#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utils.h
// 2025 - 01 - 14 (설명 추가 날짜)
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace NetworkUtil {
    inline LPFN_CONNECTEX ConnectEx;
    inline LPFN_DISCONNECTEX DisconnectEx;

    template <typename MaybeIter>
    concept IsIterator = std::contiguous_iterator<MaybeIter> or std::random_access_iterator<MaybeIter>
        or std::bidirectional_iterator<MaybeIter> or std::forward_iterator<MaybeIter>;

    template <std::contiguous_iterator Iter>
    inline typename Iter::value_type* AddressOf(Iter iter)
    {
        return &(*iter);
    }

    template <std::random_access_iterator Iter>
    inline typename Iter::value_type* AddressOf(Iter iter)
    {
        return &(*iter);
    }

    template <typename T>
    inline bool SetSocketOpt(SOCKET socket, int level, int option, T optval)
    {
        return SOCKET_ERROR != ::setsockopt(socket, level, option, reinterpret_cast<T>(optval), sizeof(optval));
    }

    inline SOCKET CreateSocket()
    {
        return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, NULL, WSA_FLAG_OVERLAPPED);
    }

    inline bool InitSockAddr(sockaddr_in& address, UINT16 port, const char* ip=nullptr)
    {
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = ::htons(port);
        if (nullptr == ip) {
            address.sin_addr.s_addr = ::htonl(INADDR_ANY);
            return true;
        }

        auto result = ::inet_pton(AF_INET, ip, &address.sin_addr.s_addr);
        return result > 0;
    }

    inline bool InitConnectExFunc(SOCKET socket)
    {
        GUID guid = WSAID_CONNECTEX;
        DWORD bytes{ };

        auto result = ::WSAIoctl(
            socket,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid,
            sizeof(guid),
            &NetworkUtil::ConnectEx,
            sizeof(NetworkUtil::ConnectEx),
            &bytes,
            nullptr,
            nullptr
        );

        return 0 == result;
    }
}
