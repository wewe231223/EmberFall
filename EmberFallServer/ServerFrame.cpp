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

ServerFrame::ServerFrame() {
    gServerCore->Init();

    mInputManager = std::make_shared<InputManager>();

    //mTimer = std::make_shared<GameTimer>();

    gServerCore->Start("", SERVER_PORT);
    auto sessionManager = gServerCore->GetSessionManager();
    sessionManager->RegisterOnSessionConnect(std::bind_front(&ServerFrame::OnPlayerConnect, this));
    sessionManager->RegisterOnSessionDisconnect(std::bind_front(&ServerFrame::OnPlayerDisconnect, this));
}

ServerFrame::~ServerFrame() { 
    mGameScenes.clear();
    gServerCore->End();
}

std::shared_ptr<class InputManager> ServerFrame::GetInputManager() const {
    return mInputManager;
}

void ServerFrame::InitGameScenes() {
    mGameScenes.emplace_back(std::make_shared<PlayScene>());

    mCurrentScene = mGameScenes.front();
    gObjectSpawner->SetCurrentScene(mCurrentScene);
    gEventManager->SetCurrentGameScene(mCurrentScene);

    mCurrentScene->Init();
    mCurrentScene->RegisterPacketProcessFunctions();

    StaticTimer::Sync(30);
}

void ServerFrame::GameLoop() {
    while (true) {
        StaticTimer::Update();
        const float deltaTime = StaticTimer::GetDeltaTime();

        mCurrentScene->DispatchPlayerEvent(mPlayerEventQueue);
        mCurrentScene->Update(deltaTime);
        mCurrentScene->LateUpdate(deltaTime);
    }
}

void ServerFrame::OnPlayerConnect(SessionIdType id) {
    auto object = std::make_shared<GameObject>(mCurrentScene);
    
    object->InitId(id);
    object->CreateCollider<OrientedBoxCollider>(BoundingBoxImporter::GetBoundingBox(EntryKeys::PLAYER_BOUNDING_BOX));
    object->CreateComponent<PlayerScript>(object, mInputManager->GetInput(id));

    Lock::SRWLockGuard playersGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, mPlayersLock };
    mPlayers[id] = object;

    PlayerEvent event{ PlayerEvent::EventType::CONNECT, id, object };
    mPlayerEventQueue.push(event);
}

void ServerFrame::OnPlayerDisconnect(SessionIdType id) {
    Lock::SRWLockGuard playersGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, mPlayersLock };

    auto it = mPlayers.find(id);
    if (it == mPlayers.end()) {
        return;
    }

    PlayerEvent event{ PlayerEvent::EventType::DISCONNECT, id, it->second };
    mPlayerEventQueue.push(event);

    mPlayers.erase(it);
    mInputManager->DeleteInput(id);
}
