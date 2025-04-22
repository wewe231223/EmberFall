#include "pch.h"
#include "GameEventManager.h"
#include "ServerFrame.h"
#include "ObjectManager.h"
#include "CollisionManager.h"
#include "Sector.h"

std::shared_ptr<GameEventManager> gEventManager = std::make_shared<GameEventManager>();
std::unique_ptr<ServerFrame> gServerFrame = std::make_unique<ServerFrame>();

std::unique_ptr<SectorSystem> gSectorSystem = std::make_unique<SectorSystem>();
std::unique_ptr<ObjectManager> gObjectManager = std::make_unique<ObjectManager>();
std::unique_ptr<CollisionManager> gCollisionManager = std::make_unique<CollisionManager>();