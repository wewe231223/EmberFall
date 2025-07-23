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
    return -mMapSize * 0.5f;
}

float Terrain::GetHeight(float x, float z, float offset) const {
    float localX = x - mHeader.minX;
    float localZ = z - mHeader.minZ;

    float fcol = localX / mHeader.gridSpacing;
    float frow = localZ / mHeader.gridSpacing;

    int col = static_cast<int>(fcol);
    int row = static_cast<int>(frow);

    col = std::clamp(col, 0, mHeader.globalWidth - 2);
    row = std::clamp(row, 0, mHeader.globalHeight - 2);

    float t = fcol - col;
    float u = frow - row;

    const SimpleMath::Vector3& v00 = mGlobalVertices[row * mHeader.globalWidth + col];
    const SimpleMath::Vector3& v10 = mGlobalVertices[row * mHeader.globalWidth + col + 1];
    const SimpleMath::Vector3& v01 = mGlobalVertices[(row + 1) * mHeader.globalWidth + col];
    const SimpleMath::Vector3& v11 = mGlobalVertices[(row + 1) * mHeader.globalWidth + col + 1];

    float y0 = v00.y * (1.0f - t) + v10.y * t;
    float y1 = v01.y * (1.0f - t) + v11.y * t;

    return y0 * (1.0f - u) + y1 * u + offset;
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

bool Terrain::Contains(const std::shared_ptr<BoundingObject>& collider, float& height) {
    if (nullptr == collider) {
        height = 0.0f;
        return false;
    }

    switch (collider->GetType()) {

    case ColliderType::SPHERE:
    {
        //auto sphereCollider = std::static_pointer_cast<SphereCollider>(collider);
        ////auto center = sphereCollider->().Center;
        ////auto radius = sphereCollider->GetBoundingSphere().Radius;
        //height = GetHeight(center);
        //return (radius + center.y) < height + MathUtil::EPSILON;
    }

    case ColliderType::ORIENTED_BOX:
    {
        auto orientedBoxCollider = std::static_pointer_cast<OBBCollider>(collider);
        auto center = orientedBoxCollider->GetBoundingBox().Center;
        auto extents = orientedBoxCollider->GetBoundingBox().Extents;
        height = GetHeight(center);
        return (extents.y - center.y) < height + MathUtil::EPSILON;
    }

    default:
        return false;
    }
}

bool Terrain::LoadFromFile(const std::filesystem::path& path) {
    std::ifstream file{ path, std::ios::binary };

    if (not file) {
        return false;
    }

    file.read(reinterpret_cast<char*>(&mHeader), sizeof(TerrainHeader));

    mGlobalVertices.resize(mHeader.globalWidth * mHeader.globalHeight);
    file.read(reinterpret_cast<char*>(mGlobalVertices.data()), mGlobalVertices.size() * sizeof(SimpleMath::Vector3));
    return true;
}
