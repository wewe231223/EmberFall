////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EmberFallServer.cpp
// 2025 - 01 - 14 김성준   : 서버 코드를 실행하기 위한 프로젝트 생성
//      - 01 - 18          : pch 추가
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "ServerFrame.h"

int main()
{
    gLogConsole->Start();
    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "Server Project Start");

    gServerFrame->Run();
    //gServerFrame->GameLoop();

    volatile bool loop{ true };
    while (loop) {};
}