#include "pch.h"
#include "GameRandom.h"

SimpleMath::Vector3 Random::GetRandVecInArea(
    const SimpleMath::Vector3& min, const SimpleMath::Vector3& max, const SimpleMath::Vector3 center) {
    
    return GetRandomVec3(min + center, max + center);
}

SimpleMath::Vector3 Random::GetRandVecInArea(
    const std::pair<SimpleMath::Vector3, SimpleMath::Vector3>& area, const SimpleMath::Vector3 center) {
    const auto& [min, max] = area;

    return GetRandomVec3(min + center, max + center);
}

SimpleMath::Vector3 Random::GetRandomColor() {
    return GetRandomDirVec3();
}

SimpleMath::Vector3 Random::GetRandomDirVec3() {
    return SimpleMath::Vector3{ GetRandom(-1.0f, 1.0f), GetRandom(-1.0f, 1.0f), GetRandom(-1.0f, 1.0f) };
}

SimpleMath::Vector2 Random::GetRandomDirVec2() {
    return SimpleMath::Vector2{ GetRandom(-1.0f, 1.0f), GetRandom(-1.0f, 1.0f) };
}

SimpleMath::Vector3 Random::GetRandomVec3(float min, float max) {
    auto randDir = GetRandomDirVec3();
    float randLength = GetRandom(min, max);
    return randDir * randLength;
}

SimpleMath::Vector3 Random::GetRandomVec3(const SimpleMath::Vector3& min, const SimpleMath::Vector3& max) {
    return SimpleMath::Vector3{ 
        GetRandom(min.x, max.x),
        GetRandom(min.y, max.y),
        GetRandom(min.z, max.z) 
    };
}

SimpleMath::Vector3 Random::GetRandomVec3(const std::pair<SimpleMath::Vector3, SimpleMath::Vector3>& area) {
    const auto& [min, max] = area;
    return SimpleMath::Vector3{
        GetRandom(min.x, max.x),
        GetRandom(min.y, max.y),
        GetRandom(min.z, max.z)
    };
}

SimpleMath::Vector2 Random::GetRandomVec2(const SimpleMath::Vector2& min, const SimpleMath::Vector2& max) {
    return SimpleMath::Vector2{
        GetRandom(min.x, max.x),
        GetRandom(min.y, max.y)
    };
}
