#pragma once

class Terrain;

namespace Path {
    inline constexpr auto DEFAULT_TILE_SIZE = 0.5m;

    enum class TileState {
        CAN_MOVE,
        CANT_MOVE,
    };

    class TileMap {
    public:
        TileMap(std::shared_ptr<Terrain> terrain);
        ~TileMap();

    public:
        TileState GetTileState(const SimpleMath::Vector3& pos);
        TileState GetTileState(size_t tileX, size_t tileZ);
        TileState GetTileState(const std::pair<size_t, size_t>& tilePos);
        std::pair<size_t, size_t> GetTileIndex(const SimpleMath::Vector3& pos);

    private:
        std::shared_ptr<Terrain> mGameMap{ };
        GameUnits::GameUnit<GameUnits::StandardLength> mTileSize{ DEFAULT_TILE_SIZE };
        std::vector<std::vector<TileState>> mTiles{ };
    };
}