#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameMath.h
// 
// 2025 - 02 - 23 : 기존 벡터, 충돌관련 함수들을 NetworkUtils와 분리함.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace MathUtil { // Ray
    struct Ray {
        SimpleMath::Vector3 origin;
        SimpleMath::Vector3 direction;
    };

    inline bool CastBox(DirectX::BoundingBox& box, Ray& ray) {

    }
}

namespace MathUtil {
    inline constexpr float SYSTEM_EPSILON = std::numeric_limits<float>::epsilon();
    inline constexpr float EPSILON = 1.0e-06f;
    inline constexpr SimpleMath::Vector3 EPSILON_VEC3 = SimpleMath::Vector3{ EPSILON };
    inline constexpr SimpleMath::Vector2 EPSILON_VEC2 = SimpleMath::Vector2{ EPSILON };

    // Absolute
    inline SimpleMath::Vector3 AbsVector(const SimpleMath::Vector3& v)
    {
        return DirectX::XMVectorAbs(v);
    }

    // Compare EPSILON Vector
    inline bool IsVectorZero(const SimpleMath::Vector3& v)
    {
        auto absVector = DirectX::XMVectorAbs(v);
        auto epsilon = DirectX::XMVectorReplicate(FLT_EPSILON);
        return DirectX::XMVector3NearEqual(DirectX::XMVectorAbs(v), SimpleMath::Vector3::Zero, EPSILON_VEC3);
    }

    // Compare v1, v2
    inline bool IsEqualVector(const SimpleMath::Vector3& v1, const SimpleMath::Vector3& v2)
    {
        return DirectX::XMVector3NearEqual(v1, v2, EPSILON_VEC3);
    }

    // Compare v1, v2 ignore Y
    inline bool IsEqualVectorXZ(const SimpleMath::Vector3& v1, const SimpleMath::Vector3& v2)
    {
        SimpleMath::Vector2 xz1{ v1.x, v1.z };
        SimpleMath::Vector2 xz2{ v2.x, v2.z };
        return DirectX::XMVector2NearEqual(xz1, xz2, EPSILON_VEC2);
    }

    // Compare Zero or Epsilon
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

    // Projection Vector to Axis
    inline float Projection(const SimpleMath::Vector3& vector, SimpleMath::Vector3 axis)
    {
        axis.Normalize();
        return axis.Dot(vector);
    }

    inline bool IsInRange(float radius, const SimpleMath::Vector3& point)
    {
        return point.LengthSquared() < (radius * radius);
    }

    inline bool IsInRange(GameUnits::GameUnit<GameUnits::Meter> radius, const SimpleMath::Vector3& point)
    {
        return point.LengthSquared() < (radius * radius).Count();
    }

    inline float VectorLengthSq(const SimpleMath::Vector3& v1, const SimpleMath::Vector3& v2)
    {
        auto sub = v1 - v2;
        return sub.LengthSquared();
    }

    inline float VectorLength(const SimpleMath::Vector3& v1, const SimpleMath::Vector3& v2)
    {
        auto sub = v1 - v2;
        return sub.Length();
    }

    inline bool IsInRange(const SimpleMath::Vector3& center, float radius, const SimpleMath::Vector3& point)
    {
        return VectorLengthSq(center, point) < (radius * radius);
    }

    inline bool IsInRange(const SimpleMath::Vector3& center, GameUnits::GameUnit<GameUnits::Meter> radius, const SimpleMath::Vector3& point)
    {
        return VectorLengthSq(center, point) < (radius * radius).Count();
    }

    // Convert Vector3 to Vector2 , Ignore Y
    inline SimpleMath::Vector2 ConvertXZVector(const SimpleMath::Vector3& v)
    {
        return SimpleMath::Vector2{ v.x, v.z };
    }

    // planeNormal은 Normalize된 벡터여야함.
    inline SimpleMath::Vector3 SlidingVector(const SimpleMath::Vector3& v, const SimpleMath::Vector3& planeNormal)
    {
        float projectionToNormal = v.Dot(planeNormal);
        SimpleMath::Vector3 slidingVec = v + (planeNormal * -projectionToNormal);
        return slidingVec;
    }
}

namespace Collision {
    using OBBPoints = std::array<SimpleMath::Vector3, 8>;
    using OBBPlaneNormals = std::array<SimpleMath::Vector3, 3>;
    using OBBEdges = std::array<SimpleMath::Vector3, 3>;

    struct ProjectionRange {
        float min;
        float max;
    };

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

    inline SimpleMath::Vector3 GetMinTransVec(const DirectX::BoundingOrientedBox& b1, const DirectX::BoundingOrientedBox& b2)
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
        if (toOpponentCenter.Dot(shortestAxis) < MathUtil::EPSILON) {
            shortestAxis = -shortestAxis;
        }

        return shortestAxis * minRange;
    }

    inline SimpleMath::Vector3 GetMinTransVec(const DirectX::BoundingSphere& s1, const DirectX::BoundingSphere& s2)
    {
        SimpleMath::Vector3 centerVec = SimpleMath::Vector3{ s1.Center } - SimpleMath::Vector3{ s2.Center };
        float distBetweenCenter = centerVec.Length();

        float overlap = distBetweenCenter - (s1.Radius + s2.Radius);
        return centerVec * overlap;
    }

    inline SimpleMath::Vector3 GetMinTransVec(const DirectX::BoundingBox& b1, const DirectX::BoundingBox& b2)
    {
        SimpleMath::Vector3 minB1 = SimpleMath::Vector3{ b1.Center } - SimpleMath::Vector3{ b1.Extents };
        SimpleMath::Vector3 maxB1 = SimpleMath::Vector3{ b1.Center } + SimpleMath::Vector3{ b1.Extents };

        SimpleMath::Vector3 minB2 = SimpleMath::Vector3{ b2.Center } - SimpleMath::Vector3{ b2.Extents };
        SimpleMath::Vector3 maxB2 = SimpleMath::Vector3{ b2.Center } + SimpleMath::Vector3{ b2.Extents };

        auto overlappedBoxMin = SimpleMath::Vector3{
            std::max(minB1.x, minB2.x),
            std::max(minB1.y, minB2.y),
            std::max(minB1.z, minB2.z)
        }; // left bottom

        auto overlappedBoxMax = SimpleMath::Vector3{
            std::min(maxB1.x, maxB2.x),
            std::min(maxB1.y, maxB2.y),
            std::min(maxB1.z, maxB2.z)
        }; // right top

        return SimpleMath::Vector3::Zero;
    }

    inline SimpleMath::Vector3 GetMinTransVec(const DirectX::BoundingOrientedBox& b1, const DirectX::BoundingBox& b2)
    {
        DirectX::BoundingOrientedBox orientedBox;
        DirectX::BoundingOrientedBox::CreateFromBoundingBox(orientedBox, b2);

        return GetMinTransVec(b1, orientedBox);
    }
}