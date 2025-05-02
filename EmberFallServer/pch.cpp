#include "pch.h"
#include "ServerFrame.h"
#include "ObjectManager.h"
#include "CollisionManager.h"
#include "GameRoom.h"
#include "Sector.h"

std::unique_ptr<ServerFrame> gServerFrame = std::make_unique<ServerFrame>();

std::unique_ptr<GameRoomManager> gGameRoomManager = std::make_unique<GameRoomManager>();
std::unique_ptr<SectorSystem> gSectorSystem = std::make_unique<SectorSystem>();
std::unique_ptr<ObjectManager> gObjectManager = std::make_unique<ObjectManager>();
std::unique_ptr<CollisionManager> gCollisionManager = std::make_unique<CollisionManager>();