#include "pch.h"
#include "TileMap.h"
#include "Terrain.h"

using namespace Path;

TileMap::TileMap(std::shared_ptr<Terrain> terrain)
    : mGameMap{ terrain } { 
    auto gameMapSize = terrain->GetMapSize();

    auto tileCount = gameMapSize / mTileSize.Count();
    mTiles.resize(tileCount.y);
    for (auto& tiles : mTiles) {
        tiles.resize(tileCount.x, TileState::CAN_MOVE);
    }
}

TileMap::~TileMap() { }

TileState Path::TileMap::GetTileState(const SimpleMath::Vector3& pos) {
    return GetTileState(GetTileIndex(pos));
}

TileState Path::TileMap::GetTileState(size_t tileX, size_t tileZ) {
    return mTiles[tileX][tileZ];
}

TileState Path::TileMap::GetTileState(const std::pair<size_t, size_t>& tilePos) {
    return mTiles[tilePos.first][tilePos.second];
}

std::pair<size_t, size_t> Path::TileMap::GetTileIndex(const SimpleMath::Vector3& pos) {
    auto tileIndex = pos / mTileSize.Count();
    return std::make_pair(static_cast<size_t>(tileIndex.x), static_cast<size_t>(tileIndex.z));
}
