#pragma once

#include "GameObjectComponent.h"

class Terrain;
class TileMap;

class GameObject;

namespace Path {
    float TIME_INFINITE = std::numeric_limits<float>::max();

    template <typename PathFindAlgorithm>
    class PathFinder : public GameObjectComponent {
    public:
        PathFinder(std::shared_ptr<TileMap> tileMap, std::shared_ptr<GameObject> owner);
        PathFinder(std::shared_ptr<Terrain> gameMap, GameUnits::GameUnit<GameUnits::Meter> tileSize, std::shared_ptr<GameObject> owner);
        virtual ~PathFinder();

    public:
        void SetMaxFindTime(const float maxTime);
        //void SetWorldMap();
        void Find();

    private:
        // 최대 탐색 시간
        // 이 시간이 지나면 이전까지 찾은 경로까지를 반환
        float mMaxFindTime{ };
        // 길찾기 알고리즘
        PathFindAlgorithm mAlgorithm;
        std::shared_ptr<TileMap> mTileMap;
        std::shared_ptr<GameObject> mOwner;
    };
}