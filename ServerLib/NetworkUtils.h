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
        return SOCKET_ERROR != ::setsockopt(socket, level, option, reinterpret_cast<T>(optval), sizeof(optval));
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

namespace MathUtil {
    using OBBPoints = std::array<SimpleMath::Vector3, 8>;
    using OBBPlaneNormals = std::array<SimpleMath::Vector3, 3>;
    using OBBEdges = std::array<SimpleMath::Vector3, 3>;

    struct ProjectionRange {
        float min;
        float max;
    };

    inline constexpr float SYSTEM_EPSILON = std::numeric_limits<float>::epsilon();
    inline constexpr float EPSILON = 1.0e-06f;
    inline constexpr SimpleMath::Vector3 EPSILON_VEC3 = SimpleMath::Vector3{ EPSILON };

    inline bool IsVectorZero(const SimpleMath::Vector3& v)
    {
        auto absVector = DirectX::XMVectorAbs(v);
        auto epsilon = DirectX::XMVectorReplicate(FLT_EPSILON);
        return DirectX::XMVector3NearEqual(DirectX::XMVectorAbs(v), SimpleMath::Vector3::Zero, EPSILON_VEC3);
    }

    inline bool IsEqualVector(const SimpleMath::Vector3& v1, const SimpleMath::Vector3& v2) 
    {
        return DirectX::XMVector3NearEqual(v1, v2, EPSILON_VEC3);
    }

    template <typename T> requires std::is_arithmetic_v<T>
    inline bool IsZero(T val)
    {
        if constexpr (std::is_integral_v<T>) {
            return 0 == val;
        }
        else {
            return std::fabs(val) < EPSILON;
        }
    }

    inline float Projection(const SimpleMath::Vector3& vector, SimpleMath::Vector3 axis)
    {
        axis.Normalize();
        return axis.Dot(vector);
    }

    inline void GetOBBPlaneNormals(const DirectX::BoundingOrientedBox& obb, OBBPlaneNormals& normals)
    {
        normals[0] = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Right, obb.Orientation);   // x
        normals[1] = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Up, obb.Orientation);      // y
        normals[2] = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Forward, obb.Orientation); // z

        for (auto& normal : normals) {
            normal.Normalize();
        }
    }

    inline void GetNonParallelEdges(const OBBPoints& points, OBBEdges& edges)
    {
        edges[0] = points[1] - points[0];
        edges[1] = points[3] - points[0];
        edges[2] = points[4] - points[0];

        for (auto& edge : edges) {
            edge.Normalize();
        }
    }

    inline void GetOBBPoints(const DirectX::BoundingOrientedBox& obb, OBBPoints& points)
    {
        points = {
            SimpleMath::Vector3{ -obb.Extents.x, -obb.Extents.y, -obb.Extents.z },  // 0 ---
            SimpleMath::Vector3{  obb.Extents.x, -obb.Extents.y, -obb.Extents.z },  // 1 +--
            SimpleMath::Vector3{  obb.Extents.x,  obb.Extents.y, -obb.Extents.z },  // 2 ++-
            SimpleMath::Vector3{ -obb.Extents.x,  obb.Extents.y, -obb.Extents.z },  // 3 -+-
            SimpleMath::Vector3{ -obb.Extents.x, -obb.Extents.y,  obb.Extents.z },  // 4 --+
            SimpleMath::Vector3{  obb.Extents.x, -obb.Extents.y,  obb.Extents.z },  // 5 +-+
            SimpleMath::Vector3{  obb.Extents.x,  obb.Extents.y,  obb.Extents.z },  // 6 +++
            SimpleMath::Vector3{ -obb.Extents.x,  obb.Extents.y,  obb.Extents.z }   // 7 -++
        };

        for (auto& point : points) {
            point = SimpleMath::Vector3::Transform(point, obb.Orientation);
            point += obb.Center;
        }
    }

    inline ProjectionRange GetProjectionRange(const OBBPoints& points, const SimpleMath::Vector3& axis)
    {
        std::array<float, 8> projection{ };
        for (size_t i = 0; i < projection.size(); ++i) {
            projection[i] = points[i].Dot(axis);
        }

        auto [min, max] = std::minmax_element(projection.begin(), projection.end());
        return { *min, *max };
    }

    inline SimpleMath::Vector3 CalcObbRepulsiveVec(const DirectX::BoundingOrientedBox& b1, const DirectX::BoundingOrientedBox& b2)
    {
        OBBPoints b1Points{ };
        GetOBBPoints(b1, b1Points); // OBB 8개 꼭짓점 얻기 (world 좌표계)

        OBBPoints b2Points{ };
        GetOBBPoints(b2, b2Points);

        OBBEdges b1Edges{ };
        GetNonParallelEdges(b1Points, b1Edges); // OBB 에서 서로 평행하지 않은 세 모서리 벡터 얻기

        OBBEdges b2Edges{ };
        GetNonParallelEdges(b2Points, b2Edges);

        OBBPlaneNormals b1Normals{ };
        GetOBBPlaneNormals(b1, b1Normals);      // OBB 에서 서로 평행하지 않은 세 면에 대한 법선벡터 얻기

        OBBPlaneNormals b2Normals{ };
        GetOBBPlaneNormals(b2, b2Normals);

        SimpleMath::Vector3 shortestAxis{ SimpleMath::Vector3::Zero };
        float minRange = FLT_MAX;
        for (auto& axis : b1Normals) { // loop 3
            auto range1 = GetProjectionRange(b1Points, axis);
            auto range2 = GetProjectionRange(b2Points, axis);

            auto overlap = ::fabs(range1.max - range2.min);
            if (overlap < minRange and false == MathUtil::IsZero(overlap)) {
                shortestAxis = axis;
                minRange = overlap;
            }
        }

        for (auto& axis : b2Normals) { // loop 3
            auto range1 = GetProjectionRange(b1Points, axis);
            auto range2 = GetProjectionRange(b2Points, axis);

            auto overlap = ::fabs(range1.max - range2.min);
            if (overlap < minRange and false == MathUtil::IsZero(overlap)) {
                shortestAxis = axis;
                minRange = overlap;
            }
        }

        for (auto& edge1 : b1Edges) { // loop 9 (3 * 3)
            for (auto& edge2 : b2Edges) {
                auto axis = edge1.Cross(edge2);
                axis.Normalize();

                auto range1 = GetProjectionRange(b1Points, axis);
                auto range2 = GetProjectionRange(b2Points, axis);

                auto overlap = ::fabs(range1.max - range2.min);
                if (overlap < minRange and false == MathUtil::IsZero(overlap)) {
                    shortestAxis = axis;
                    minRange = overlap;
                }
            }
        }

        SimpleMath::Vector3 c1 = b1.Center;
        SimpleMath::Vector3 c2 = b2.Center;
        auto toOpponentCenter = c1 - c2;
        // b2 에서 b1를 향하도록 벡터의 방향을 조정 (Dot결과에 따라서 부호 변경 (방향만 반대인 평행 벡터)
        if (toOpponentCenter.Dot(shortestAxis) < EPSILON) {
            shortestAxis = -shortestAxis;
        }

        return shortestAxis * minRange;
    }
}