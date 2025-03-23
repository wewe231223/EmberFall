#include "pch.h"
#include "Terrain.h"
#include "Collider.h"

Terrain::Terrain(const std::filesystem::path& path) {
    LoadFromFile(path);
}

Terrain::~Terrain() { }

SimpleMath::Vector2 Terrain::GetMapSize() const {
    return mMapSize;
}

SimpleMath::Vector2 Terrain::GetMapLeftBottom() const {
    return -mMapSize / 2.0f;
}

float Terrain::GetHeight(float x, float z, float offset) const {
    float localX = x - mMinX;
    float localZ = z - mMinZ;

    float fcol = localX / mGridSpacing;
    float frow = localZ / mGridSpacing;

    int col = static_cast<int>(fcol);
    int row = static_cast<int>(frow);

    col = std::clamp(col, 0, mGlobalWidth - 2);
    row = std::clamp(row, 0, mGlobalHeight - 2);

    float t = fcol - col;
    float u = frow - row;

    decltype(auto) v00 = mGlobalVertices[row * mGlobalWidth + col];
    decltype(auto) v10 = mGlobalVertices[row * mGlobalWidth + col + 1];
    decltype(auto) v01 = mGlobalVertices[(row + 1) * mGlobalWidth + col];
    decltype(auto) v11 = mGlobalVertices[(row + 1) * mGlobalWidth + col + 1];

    float y0 = v00.y * (1.0f - t) + v10.y * t;
    float y1 = v01.y * (1.0f - t) + v11.y * t;

    return y0 * (1.0f - u) + y1 * u;
}

float Terrain::GetHeight(const SimpleMath::Vector2& pos, float offset) const {
    return GetHeight(pos.x, pos.y, offset);
}

float Terrain::GetHeight(const SimpleMath::Vector3& pos, float offset) const {
    return GetHeight(pos.x, pos.z, offset);
}

bool Terrain::Contains(const SimpleMath::Vector3& position) {
    return position.y <= GetHeight(position);
}

bool Terrain::Contains(const std::shared_ptr<Collider>& collider, float& height) {
    switch (collider->GetType()) {
    case ColliderType::BOX:
        {
            auto a = std::static_pointer_cast<BoxCollider>(collider);
            auto center = a->GetBoundingBox().Center;
            height = GetHeight(center, a->GetBoundingBox().Extents.y) + MathUtil::EPSILON;
            return center.y < height;
        }

    case ColliderType::SPHERE:
        {
            auto a = std::static_pointer_cast<SphereCollider>(collider);
            auto center = a->GetBoundingSphere().Center;
            height = GetHeight(center, a->GetBoundingSphere().Radius);
            return center.y < height;
        }

    case ColliderType::ORIENTED_BOX:
        {
            auto a = std::static_pointer_cast<OrientedBoxCollider>(collider);
            auto center = a->GetBoundingBox().Center;
            height = GetHeight(center, a->GetBoundingBox().Extents.y) + MathUtil::EPSILON;
            return center.y < height;
        }

    default:
        return false;
    }
}

bool Terrain::LoadFromFile(const std::filesystem::path& path) {
    std::ifstream file{ path, std::ios::binary };

    if (!file) {
        return false;
    }

    file.read(reinterpret_cast<char*>(&mGlobalWidth), sizeof(mGlobalWidth));
    file.read(reinterpret_cast<char*>(&mGlobalHeight), sizeof(mGlobalHeight));
    file.read(reinterpret_cast<char*>(&mGridSpacing), sizeof(mGridSpacing));
    file.read(reinterpret_cast<char*>(&mMinX), sizeof(mMinX));
    file.read(reinterpret_cast<char*>(&mMinZ), sizeof(mMinZ));

    mGlobalVertices.resize(mGlobalWidth * mGlobalHeight);
    file.read(reinterpret_cast<char*>(mGlobalVertices.data()), mGlobalVertices.size() * sizeof(SimpleMath::Vector3));

    return true;
}
