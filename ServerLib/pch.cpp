#include "pch.h"

std::unique_ptr<LogConsole> gLogConsole = std::make_unique<LogConsole>();
std::shared_ptr<ClientCore> gClientCore2 = std::make_shared<ClientCore>();