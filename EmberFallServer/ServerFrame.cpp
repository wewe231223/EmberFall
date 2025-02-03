#include "pch.h"
#include "ServerFrame.h"
#include "ServerGameScene.h"

ServerFrame::ServerFrame() {
    mServerCore = std::make_shared<ServerCore> ();
    mServerCore->Init();

    mServerCore->Start("127.0.0.1", SERVER_PORT);
}

ServerFrame::~ServerFrame() { 
    mGameScenes.clear();
    mServerCore->End();
}

void ServerFrame::InitGameScenes() {
    mGameScenes.emplace_back(std::make_shared<PlayScene>());

    mCurrentScene = mGameScenes.front();
    mCurrentScene->RegisterOnSessionConnect(mServerCore);
}

void ServerFrame::GameLoop() {
    while (true) {
        std::this_thread::sleep_for(100ms);
        mCurrentScene->ProcessPackets(mServerCore);
        mCurrentScene->Update(0.0f); // Temp
        mCurrentScene->SendUpdateResult(mServerCore);
    }
}
