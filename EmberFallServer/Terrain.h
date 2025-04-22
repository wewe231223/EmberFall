#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Terrain.h
//  2024 - 02 - 04  : 실외지형 표현
//                모든 좌표는 float으로 표현할 것이므로 float 좌표를 기준으로 높이를 얻음
//                float으로 높이를 얻을 경우 점을 포함하는 사각형 각각의 높이를 보간하여 최종 높이를 얻는다.
// 
//         03 - 21 : 클라이언트에서 생성한 테셀레이션 된 실외지형 파일을 불러오고 테셀레이션 된 지형에
//                  맞는 높이를 반환하도록 수정
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

using PixelType = BYTE;

class Terrain {
public:
    Terrain(const std::filesystem::path& path);
    ~Terrain();

    //Terrain(const Terrain& other);
    //Terrain(Terrain&& other) noexcept;
    //Terrain& operator=(const Terrain& other);
    //Terrain& operator=(Terrain&& other) noexcept;

public:
    SimpleMath::Vector2 GetMapSize() const;
    SimpleMath::Vector2 GetMapLeftBottom() const;

    float GetHeight(float x, float z, float offset = 0.0f) const;
    float GetHeight(const SimpleMath::Vector2& pos, float offset = 0.0f) const;
    float GetHeight(const SimpleMath::Vector3& pos, float offset = 0.0f) const;

    bool Contains(const SimpleMath::Vector3& position);
    bool Contains(const std::shared_ptr<class BoundingObject>& collider, float& height);

private:
    bool LoadFromFile(const std::filesystem::path& path);

private:
    SimpleMath::Vector2 mMapSize{ GameProtocol::Map::STAGE1_MAP_WIDTH.Count() , GameProtocol::Map::STAGE1_MAP_HEIGHT.Count()};

    std::vector<SimpleMath::Vector3> mGlobalVertices;
    int32_t  mGlobalWidth{ };
    int32_t  mGlobalHeight{ };
    float mGridSpacing{ };
    float mMinX{ };
    float mMinZ{ };
};
