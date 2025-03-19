#include "pch.h"
#include "GameEventManager.h"
#include "ServerFrame.h"

std::shared_ptr<GameEventManager> gEventManager = std::make_shared<GameEventManager>();
std::unique_ptr<ServerFrame> gServerFrame = std::make_unique<ServerFrame>();
