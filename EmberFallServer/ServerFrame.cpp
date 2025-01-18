#include "pch.h"
#include "ServerFrame.h"
#include "ServerGameScene.h"

ServerFrame::ServerFrame() 
    : mServerCore{ std::make_unique<ServerCore>() } { 
    mServerCore->Start(SERVER_PORT);
}

ServerFrame::~ServerFrame() { 
    mGameScenes.clear();
    mServerCore->End();
}

void ServerFrame::InitGameScenes() {
    mGameScenes.emplace_back(std::make_shared<EchoTestScene>());

    mCurrentScene = mGameScenes.front();
}

void ServerFrame::GameLoop() {
    while (true) {
        mCurrentScene->ProcessPackets();
        mCurrentScene->Update(0.0f); // Temp
    }
}
