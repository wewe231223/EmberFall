////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EmberFallServer.cpp
// 2025 - 01 - 14 김성준: 서버 코드를 실행하기 위한 프로젝트 생성
//                      
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../ServerLib/pch.h"
#include "../ServerLib/ServerCore.h"
#include "../ServerLib/PacketHandler.h"
#include "../ServerLib/SessionManager.h"

#pragma comment(lib, "../out/debug/ServerLib.lib")

int main()
{
    ServerCore core;
    core.Start("127.0.0.1", 7777);

    volatile bool test{ true };
    while (test) {
        auto& buffer = gPacketHandler->GetBuffer();
        if (0 == buffer.Size()) {
            continue;
        }

        std::cout << "Total RecvBytes: " << buffer.Size() << std::endl;
        std::cout << "Contents: " << buffer.Data() << std::endl;

        gSessionManager->SendAll(buffer.Data(), buffer.Size());
    }

    core.End();
}