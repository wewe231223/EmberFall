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

    template <typename T>
    inline bool SetSocketOpt(SOCKET socket, int level, int option, T optval)
    {
        return SOCKET_ERROR != ::setsockopt(socket, level, option, reinterpret_cast<const char*>(&optval), sizeof(optval));
    }

    inline std::string WSAErrorMessage(const std::source_location& sl = std::source_location::current())
    {
        auto errorCode = WSAGetLastError();

        LPVOID msg;
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<char*>(&msg),
            0,
            NULL
        );

        auto str = std::format("Error Occurred!\n\nFILE: {}\n\nFUNCTION: {}\n\nLINE: {}\n\nError Code: {}\n\nError: {}",
            sl.file_name(), sl.function_name(), sl.line(), errorCode, reinterpret_cast<char*>(msg));

        LocalFree(msg);

        return str;
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

    template <std::contiguous_iterator Iter> requires std::is_same_v<typename Iter::value_type, char>
    inline PacketSizeType GetPacketSizeFromIter(Iter iter)
    {
        return *(reinterpret_cast<PacketSizeType*>(AddressOf(iter)));
    }
}