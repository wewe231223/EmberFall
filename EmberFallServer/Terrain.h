#pragma once

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
    Terrain(std::string_view imageFile, const SimpleMath::Vector3& scale=SimpleMath::Vector3::One, size_t imageWidth = 0, size_t imageHeight = 0);
    Terrain(std::shared_ptr<HeightMap> image, const SimpleMath::Vector3& scale=SimpleMath::Vector3::One);
    ~Terrain();

    Terrain(const Terrain& other);
    Terrain(Terrain&& other) noexcept;
    Terrain& operator=(const Terrain& other);
    Terrain& operator=(Terrain&& other) noexcept;

public:
    std::shared_ptr<HeightMap> GetHeightMap() const;

    void ResetHeightMap(std::shared_ptr<HeightMap> heightMap = nullptr);
    void ResetScale(const SimpleMath::Vector3& scale = SimpleMath::Vector3::One);

    float GetHeight(const SimpleMath::Vector2& pos, float offset = 0.0f) const;
    float GetHeight(const SimpleMath::Vector3& pos, float offset = 0.0f) const;

private:
    SimpleMath::Vector3 mScale{ };
    std::shared_ptr<HeightMap> mHeightMap{ };
};
