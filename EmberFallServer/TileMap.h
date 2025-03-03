#pragma once

class Terrain;

namespace Path {
    enum class TileState {
        CAN_MOVE,
        CANT_MOVE,
    };

    class TileMap {
    public:
        static constexpr auto DEFAULT_TILE_SIZE = 0.5m;

    public:
        TileMap(std::shared_ptr<Terrain> terrain);
        ~TileMap();

    public:
        void Update();

    private:
        std::shared_ptr<Terrain> mGameMap;
        GameUnits::GameUnit<GameUnits::StandardLength> mTileSize{ DEFAULT_TILE_SIZE };
    };
}