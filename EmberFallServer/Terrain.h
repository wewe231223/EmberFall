#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Terrain.h
//  2024 - 02 - 04 김성준 : 실외지형 표현
//                모든 좌표는 float으로 표현할 것이므로 float 좌표를 기준으로 높이를 얻음
//                float으로 높이를 얻을 경우 점을 포함하는 사각형 각각의 높이를 보간하여 최종 높이를 얻는다.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

using PixelType = BYTE;

class HeightMap {
public:
    HeightMap() = delete;

    HeightMap(std::string_view imageFile, size_t imageWidth=0, size_t imageHeight=0);
    ~HeightMap();

    HeightMap(const HeightMap& other);
    HeightMap(HeightMap&& other) noexcept;
    HeightMap& operator=(const HeightMap& other);
    HeightMap& operator=(HeightMap&& other) noexcept;

public:
    size_t ImageWidth() const;
    size_t ImageHeight() const;

    float GetPixel(size_t idx) const;
    float GetPixel(size_t x, size_t y) const;
    float GetPixel(const float x, const float y) const;
    float GetPixel(const SimpleMath::Vector2& uv) const;
    size_t Size() const;
    size_t PixelCount() const;

private:
    size_t mWidth{ };
    size_t mHeight{ };
    std::vector<PixelType> mPixels{ };
};

class Terrain {
public:
    Terrain(std::string_view imageFile, const SimpleMath::Vector2& mapSize = { 1000.0f, 1000.0f }, size_t imageWidth = 0, size_t imageHeight = 0);
    Terrain(std::shared_ptr<HeightMap> image, const SimpleMath::Vector3& scale=SimpleMath::Vector3::One);
    ~Terrain();

    Terrain(const Terrain& other);
    Terrain(Terrain&& other) noexcept;
    Terrain& operator=(const Terrain& other);
    Terrain& operator=(Terrain&& other) noexcept;

public:
    std::shared_ptr<HeightMap> GetHeightMap() const;

    void ResetHeightMap(std::shared_ptr<HeightMap> heightMap = nullptr);

    SimpleMath::Vector2 GetMapSize() const;
    std::pair<SimpleMath::Vector2, SimpleMath::Vector2> GetArea() const;

    float GetHeight(const SimpleMath::Vector2& pos, float offset = 0.0f) const;
    float GetHeight(const SimpleMath::Vector3& pos, float offset = 0.0f) const;

    bool Contains(const SimpleMath::Vector3& position);
    bool Contains(const std::shared_ptr<class Collider>& collider, float& height);

private:
    float mScaleY{ 0.3f };
    SimpleMath::Vector2 mMapSize{ };
    SimpleMath::Vector2 mTileSize{ };
    SimpleMath::Vector2 mLeftBottom{ };
    std::shared_ptr<HeightMap> mHeightMap{ };
};
