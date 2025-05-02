#include "pch.h"
#include "ServerFrame.h"
#include "GameRoom.h"

std::unique_ptr<ServerFrame> gServerFrame = std::make_unique<ServerFrame>();

std::unique_ptr<GameRoomManager> gGameRoomManager = std::make_unique<GameRoomManager>();