#pragma once

class TileMap;

namespace Path {
    float TIME_INFINITE = std::numeric_limits<float>::max();

    template <typename PathFindAlgorithm>
    class PathFinder {
    public:
        PathFinder();
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
    };
}