#pragma once

#include "GraphBase.h"
#include "Terrain.h"

namespace Graphs {
    class GraphMap {
    public:
        using NodeIdx = Graph::IndexType;
        
        GameUnits::GameUnit<GameUnits::Meter> DEFAULT_CELL_SIZE = 0.5f;

    public:
        GraphMap(const std::shared_ptr<Terrain>& terrain);

        size_t ConvertPositionToIndex(const SimpleMath::Vector3& pos);
        SimpleMath::Vector3 ConvertIndexToPosition(NodeIdx idx);

    private:
        Graph mGraph;
        NodeIdx mMaxIdx{ };
        SimpleMath::Vector2 mMapSize{ };
        SimpleMath::Vector2 mMapLeftBottom{ };
    };
}