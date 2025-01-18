#include "pch.h"
#include "Constants.h"
#include "IOCPCore.h"
#include "SessionManager.h"
#include "PacketHandler.h"
#include "SendBuffers.h"

std::unique_ptr<IOCPCore> gIocpCore = std::make_unique<IOCPCore>();
std::unique_ptr<SessionManager> gSessionManager = std::make_unique<SessionManager>();
std::unique_ptr<PacketHandler> gPacketHandler = std::make_unique<PacketHandler>();
std::unique_ptr<SendBufferFactory> gSendBufferFactory = std::make_unique<SendBufferFactory>();