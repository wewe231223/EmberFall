#include "pch.h"
#include "GraphMap.h"

using namespace Graphs;

Graphs::GraphMap::GraphMap(const std::shared_ptr<Terrain>& terrain) {
    mMapSize = terrain->GetMapSize() / DEFAULT_CELL_SIZE.Count();
    mMapLeftBottom = terrain->GetMapLeftBottom();
    size_t mapWidth = static_cast<size_t>(mMapSize.x);
    size_t mapHeight = static_cast<size_t>(mMapSize.y);
    mMaxIdx = static_cast<NodeIdx>(mapWidth * mapHeight);

    // 방향 벡터 (상, 하, 좌, 우, 대각선)
    constexpr Graph::IndexType dx[8] = { 0,  0, -1,  1, -1, -1,  1,  1 };
    constexpr Graph::IndexType dy[8] = { -1, 1,  0,  0, -1,  1, -1,  1 };

    mGraph.Reserve(mapWidth * mapHeight);
    for (size_t y = 0; y < mapHeight; ++y) {
        for (size_t x = 0; x < mapWidth; ++x) {
            Graph::IndexType currNode = static_cast<Graph::IndexType>(mapWidth * y + x);

            for (Graph::IndexType dir = 0; dir < 8; ++dir) {
                int nx = static_cast<int>(x) + dx[dir];
                int ny = static_cast<int>(y) + dy[dir];

                if (nx >= 0 and nx < static_cast<int>(mapWidth)
                    and ny >= 0 and ny < static_cast<int>(mapHeight)) {

                    size_t neighborNode = mapWidth * ny + nx;
                    mGraph.AddEdge(currNode, static_cast<Graph::IndexType>(neighborNode));
                }
            }
        }
    }
}

size_t Graphs::GraphMap::ConvertPositionToIndex(const SimpleMath::Vector3& pos) {
    auto cellPos = pos / DEFAULT_CELL_SIZE.Count();
    
    // round
    NodeIdx idx = std::lround(cellPos.z) * static_cast<INT32>(mMapSize.x) + std::lround(cellPos.x);
    if (idx < 0 or idx > mMaxIdx) {
        return Graph::INVALID_NODE;
    }
    return idx;
}

SimpleMath::Vector3 Graphs::GraphMap::ConvertIndexToPosition(NodeIdx idx) {
    float nodeX = static_cast<float>(idx % static_cast<NodeIdx>(mMapSize.x));
    float nodeY = static_cast<float>(idx / static_cast<NodeIdx>(mMapSize.y));

    return mMapLeftBottom + SimpleMath::Vector3{ nodeX * DEFAULT_CELL_SIZE.Count(), 0.0f, nodeY * DEFAULT_CELL_SIZE.Count() };
}
