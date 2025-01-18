#pragma once

#include "../ServerLib/pch.h"
#include "../ServerLib/ServerCore.h"
#include "../ServerLib/PacketHandler.h"

#pragma comment(lib, "../out/Debug/ServerLib.lib")

inline constexpr UINT16 SERVER_PORT = 7777;

class ServerFrame {
public:
    ServerFrame();
    ~ServerFrame();

public:
    void GameLoop();

private:
    bool mDone{ };
    std::unique_ptr<class ServerCore> mServerCore{ nullptr };
};