#pragma once

#include "GameObjectComponent.h"
#include "GraphMap.h"
#include "GraphSearch.h"

class GameObject;

namespace Path {
    float TIME_INFINITE = std::numeric_limits<float>::max();

    template <typename PathFindAlgorithm>
    class PathFinder : public GameObjectComponent {
    public:
        PathFinder(std::shared_ptr<Graphs::GraphMap> graphMap, std::shared_ptr<GameObject> owner);
        PathFinder(std::shared_ptr<Terrain> terrain, std::shared_ptr<GameObject> owner);
        virtual ~PathFinder();

    public:
        void Find();

    private:
        // 최대 탐색 시간
        // 이 시간이 지나면 이전까지 찾은 경로까지를 반환
        float mMaxFindTime{ };
        // 길찾기 알고리즘
        PathFindAlgorithm mAlgorithm;
        std::shared_ptr<Graphs::GraphMap> mTileMap;
        std::shared_ptr<GameObject> mOwner;
    };

    template<typename PathFindAlgorithm>
    inline PathFinder<PathFindAlgorithm>::PathFinder(std::shared_ptr<Graphs::GraphMap> graphMap, std::shared_ptr<GameObject> owner)
        : mTileMap{ graphMap }, mOwner{ owner } { }

    template<typename PathFindAlgorithm>
    inline PathFinder<PathFindAlgorithm>::PathFinder(std::shared_ptr<Terrain> terrain, std::shared_ptr<GameObject> owner) 
        : mTileMap{ std::make_shared<Graphs::GraphMap>(terrain) }, mOwner{owner} { }

    template<typename PathFindAlgorithm>
    inline PathFinder<PathFindAlgorithm>::~PathFinder() { }

    template<typename PathFindAlgorithm>
    inline void PathFinder<PathFindAlgorithm>::Find() {
        Graphs::Search::SearchState state = mAlgorithm.CycleOnce();
    }
}