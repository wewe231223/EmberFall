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
    gServerCore->End();
}

std::shared_ptr<class InputManager> ServerFrame::GetInputManager() const {
    return mInputManager;
}

void ServerFrame::Run() {
    BoundingBoxImporter::LoadFromFile();

    gServerCore->Init();

    mInputManager = std::make_shared<InputManager>();

    auto sessionManager = gServerCore->GetSessionManager();
    static auto createSessionFn = []() -> std::shared_ptr<Session> { 
        return std::make_shared<GameSession>(gServerCore);
    };

    sessionManager->RegisterCreateSessionFn(createSessionFn);

    gServerCore->Start("", SERVER_PORT);

    while (not mDone) { };
}