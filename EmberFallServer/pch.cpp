#include "pch.h"
#include "GameEventManager.h"
#include "ServerFrame.h"
#include "ObjectSpawner.h"
#include "ObjectManager.h"
#include "Sector.h"

std::shared_ptr<GameEventManager> gEventManager = std::make_shared<GameEventManager>();
std::shared_ptr<ServerCore> gServerCore = std::make_shared<ServerCore>();
std::unique_ptr<ServerFrame> gServerFrame = std::make_unique<ServerFrame>();
std::unique_ptr<ObjectSpawner> gObjectSpawner = std::make_unique<ObjectSpawner>();
std::unique_ptr<ObjectManager> gObjectManager = std::make_unique<ObjectManager>();
std::unique_ptr<SectorSystem> gSectorSystem = std::make_unique<SectorSystem>();