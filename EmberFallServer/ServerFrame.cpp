#include "pch.h"
#include "ServerFrame.h"
#include "ServerGameScene.h"
#include "GameTimer.h"
#include "GameObject.h"
#include "ObjectSpawner.h"
#include "GameEventManager.h"
#include "BoundingBoxImporter.h"
#include "Input.h"

#include "PlayerScript.h"
#include "GameSession.h"

ServerFrame::ServerFrame() {}

ServerFrame::~ServerFrame() { 
    mGameScenes.clear();
    gServerCore->End();
}

std::shared_ptr<class InputManager> ServerFrame::GetInputManager() const {
    return mInputManager;
}

void ServerFrame::Run() {
    gServerCore->Init();

    mInputManager = std::make_shared<InputManager>();

    auto sessionManager = gServerCore->GetSessionManager();
    auto fn = []() -> std::shared_ptr<Session> { return std::make_shared<GameSession>(gServerCore); };
    sessionManager->RegisterCreateSessionFn(fn);

    gServerCore->Start("", SERVER_PORT);

    while (not mDone) { };
}

void ServerFrame::InitGameScenes() {
    BoundingBoxImporter::LoadFromFile();
}