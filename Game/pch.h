#pragma once 
#include <memory>
#include "../ServerLib/pch.h"
#include "../ServerLib/NetworkCore.h"

#ifdef _DEBUG
#pragma comment(lib, "out/debug/ServerLib.lib")
#else 
#pragma comment(lib, "out/release/ServerLib.lib")
#endif 

extern std::shared_ptr<ClientCore> gClientCore;