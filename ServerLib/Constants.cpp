#include "pch.h"
#include "Constants.h"
#include "IOCPCore.h"
#include "SessionManager.h"

std::unique_ptr<IOCPCore> gIocpCore = std::make_unique<IOCPCore>();
std::unique_ptr<SessionManager> gSessionManager = std::make_unique<SessionManager>();