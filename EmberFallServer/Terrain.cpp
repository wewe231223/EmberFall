#include "pch.h"
#include "Terrain.h"
#include "Collider.h"

HeightMap::HeightMap(std::string_view imageFilePath, size_t imageWidth, size_t imageHeight) {
    std::filesystem::path path{ imageFilePath };
    size_t fileSize = 0;
    if (0 == imageWidth or 0 == imageHeight) {
        fileSize = std::filesystem::file_size(imageFilePath);
        mWidth = mHeight = static_cast<size_t>(std::sqrt(fileSize));
    }
    else {
        mWidth = imageWidth;
        mHeight = imageHeight;
        fileSize = imageWidth * imageHeight;
    }

    std::ifstream imageFile{ path, std::ios::binary };
    if (not imageFile.is_open()) {
        return;
    }

    mPixels.resize(fileSize / sizeof(PixelType));
    imageFile.read(reinterpret_cast<char*>(mPixels.data()), fileSize);

    std::cout << std::format("Height Map Load Success [File: {}] [Size: {}]\n", imageFilePath, fileSize);
    std::cout << std::format("Height Map Info [Width: {}] [Height: {}]\n", mWidth, mHeight);
}

HeightMap::~HeightMap() { }

HeightMap::HeightMap(const HeightMap& other) {
    mPixels.resize(other.PixelCount());
    std::copy(other.mPixels.begin(), other.mPixels.end(), mPixels.begin());
}

HeightMap::HeightMap(HeightMap&& other) noexcept {
    mPixels = std::move(other.mPixels);
}

HeightMap& HeightMap::operator=(const HeightMap& other) {
    mPixels.resize(other.PixelCount());
    std::copy(other.mPixels.begin(), other.mPixels.end(), mPixels.begin());
    return *this;
}

HeightMap& HeightMap::operator=(HeightMap&& other) noexcept {
    mPixels = std::move(other.mPixels);
    return *this;
}

size_t HeightMap::ImageWidth() const {
    return mWidth;
}

size_t HeightMap::ImageHeight() const {
    return mHeight;
}

float HeightMap::GetPixel(size_t idx) const {
    return static_cast<float>(mPixels[idx]);
}

float HeightMap::GetPixel(size_t u, size_t v) const {
    return static_cast<float>(mPixels[mWidth * v + u]);
}

float HeightMap::GetPixel(const float u, const float v) const {
    size_t iu{ static_cast<size_t>(u) };
    size_t iv{ static_cast<size_t>(v) };

    if ((iu >= mWidth - 1 or iu >= mHeight - 1) or (0.0f > u or 0.0f > v)) {
        return 0.0f;
    }

    float fu{ u - iu };
    float fv{ v - iv };

    float ltHeight{ GetPixel(iu, iv + 1) };
    float rtHeight{ GetPixel(iu + 1, iv + 1) };
    float lbHeight{ GetPixel(iu, iv) };
    float rbHeight{ GetPixel(iu + 1, iv) };

    float topHeight = std::lerp(ltHeight, rtHeight, fu);
    float bottomHeight = std::lerp(ltHeight, rtHeight, fu);

    return std::lerp(topHeight, bottomHeight, fv);
}

float HeightMap::GetPixel(const SimpleMath::Vector2& uv) const {
    return GetPixel(uv.x, uv.y);
}

size_t HeightMap::Size() const {
    return mPixels.size() * sizeof(PixelType);
}

size_t HeightMap::PixelCount() const {
    return mPixels.size();
}

Terrain::Terrain(std::string_view imageFile, const SimpleMath::Vector3& scale, size_t imageWidth, size_t imageHeight) 
    : mScale{ scale } {
    mHeightMap = std::make_shared<HeightMap>(imageFile, imageWidth, imageHeight);
}

Terrain::Terrain(std::shared_ptr<HeightMap> image, const SimpleMath::Vector3& scale) 
    : mScale{ scale }, mHeightMap{ image } { }

Terrain::~Terrain() { }

Terrain::Terrain(const Terrain& other) { }

Terrain::Terrain(Terrain&& other) noexcept { }

Terrain& Terrain::operator=(const Terrain& other) {
    return *this;
}

Terrain& Terrain::operator=(Terrain&& other) noexcept {
    return *this;
}

void Terrain::ResetHeightMap(std::shared_ptr<HeightMap> heightMap) {
    mHeightMap = heightMap;
}

void Terrain::ResetScale(const SimpleMath::Vector3& scale) {
    mScale = scale;
}

std::shared_ptr<HeightMap> Terrain::GetHeightMap() const {
    return mHeightMap;
}

float Terrain::GetHeight(const SimpleMath::Vector2& pos, float offset) const {
    float idxX = pos.x / mScale.x;
    float idxZ = pos.y / mScale.z;

    float pixel = mHeightMap->GetPixel(idxX, idxZ);
    return pixel * mScale.y + offset;
}

float Terrain::GetHeight(const SimpleMath::Vector3& pos, float offset) const {
    float idxX = pos.x / mScale.x;
    float idxZ = pos.z / mScale.z;

    float pixel = mHeightMap->GetPixel(idxX, idxZ);
    return pixel * mScale.y + offset;
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
        height = GetHeight(center, -a->GetBoundingBox().Extents.y);
        return center.y < height;
    }

    case ColliderType::SPHERE:
    {
        auto a = std::static_pointer_cast<SphereCollider>(collider);
        auto center = a->GetBoundingSphere().Center;
        height = GetHeight(center, -a->GetBoundingSphere().Radius);
        return center.y < height;
        
    }

    case ColliderType::ORIENTED_BOX:
    {
        auto a = std::static_pointer_cast<OrientedBoxCollider>(collider);
        auto center = a->GetBoundingBox().Center;
        height = GetHeight(center, -a->GetBoundingBox().Extents.y);
        return center.y < height;
    }

    default:
        return false;
    }
}
