#include "pch.h"
#include "GameEventManager.h"
#include "ServerFrame.h"
#include "ObjectSpawner.h"

std::shared_ptr<GameEventManager> gEventManager = std::make_shared<GameEventManager>();
std::unique_ptr<ServerFrame> gServerFrame = std::make_unique<ServerFrame>();
std::unique_ptr<ObjectSpawner> gObjectSpawner = std::make_unique<ObjectSpawner>();
