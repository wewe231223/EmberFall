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

    class Serializer;

    template <std::contiguous_iterator Iter> requires std::is_same_v<typename Iter::value_type, char>
    inline PacketSizeType GetPacketSizeFromIter(Iter iter)
    {
        auto size = *(reinterpret_cast<PacketSizeType*>(AddressOf(iter)));
        Serializer::Deserialize(&size);
        return size;
    }

    inline NetworkObjectIdType ObjectIdToIndexId(NetworkObjectIdType objectId)
    {
        return objectId - OBJECT_ID_START;
    }

    class Serializer {
    public:
        // 05-13 김성준 추가 : 직렬화 함수는 산술타입의 주소를 인자로 받는다.
        template <typename From> requires std::is_arithmetic_v<From>
        static void Serialize(From* value)
        {
            if constexpr (NATIVE_ENDIAN == FBS_ENDIAN or sizeof(From) == sizeof(char)) {
                return;
            }
            else {
                if constexpr (std::is_integral_v<From>) {
                    From data{ };

                    // From 타입에 맞는 바이트 정렬
                    if constexpr (sizeof(From) == sizeof(short)) {
                        data = ::htons(*value);
                    }
                    else if constexpr (sizeof(From) == sizeof(long)) {
                        data = ::htonl(*value);
                    }
                    else if constexpr (sizeof(From) == sizeof(long long)) {
                        data = ::htonll(*value);
                    }

                    memcpy(value, &data, sizeof(From));
                }
                else if constexpr (std::is_floating_point_v<From>) {
                    if constexpr (sizeof(From) == sizeof(float)) {
                        unsigned int data = ::htonf(*value);
                        memcpy(value, &data, sizeof(From));
                    }
                    else if constexpr (sizeof(From) == sizeof(double)) {
                        unsigned long long data = ::htond(*value);
                        memcpy(value, &data, sizeof(From));
                    }
                }
            }
        }

        // 05-13 김성준 추가 : 역직렬화 함수는 산술 타입의 주소를 인자로 받는다.
        //					  해당 산술타입의 값을 호스트의 바이트 정렬 방식으로 바꾸어
        //					  해당 값의 주소에 복사한다.
        template <typename To> requires std::is_arithmetic_v<To>
        static void Deserialize(To* data)
        {
            if constexpr (NATIVE_ENDIAN == FBS_ENDIAN) {
                return;
            }
            else {
                if constexpr (std::is_integral_v<To>) {
                    To value{ };
                    if constexpr (sizeof(To) == sizeof(short)) {
                        value = ::ntohs(*data);
                    }
                    else if constexpr (sizeof(To) == sizeof(long)) {
                        value = ::ntohl(*data);
                    }
                    else if constexpr (sizeof(To) == sizeof(long long)) {
                        value = ::ntohll(*data);
                    }

                    memcpy(data, &value, sizeof(To));
                }
                else if constexpr (std::is_floating_point_v<To>) {
                    To value{ };
                    if constexpr (sizeof(To) == sizeof(float)) {
                        unsigned int integralData{ };
                        memcpy(&integralData, data, sizeof(To));

                        value = ::ntohf(integralData);
                    }
                    else if constexpr (sizeof(To) == sizeof(double)) {
                        unsigned long long integralData{ };
                        memcpy(&integralData, data, sizeof(To));

                        value = ::ntohd(integralData);
                    }

                    memcpy(data, &value, sizeof(To));
                }
            }
        }
    };
}