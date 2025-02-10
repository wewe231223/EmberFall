#include "pch.h"
#include "ServerFrame.h"
#include "ServerGameScene.h"
#include "GameTimer.h"
#include "GameObject.h"
#include "Input.h"

#include "PlayerScript.h"

#define RAND_COLOR static_cast<float>(rand()) / static_cast<float>(RAND_MAX) // TEST

ServerFrame::ServerFrame() {
    mServerCore = std::make_shared<ServerCore>();
    mServerCore->Init();

    mInputManager = std::make_shared<InputManager>();

    mTimer = std::make_shared<GameTimer>();

    mServerCore->Start("127.0.0.1", SERVER_PORT);
    auto sessionManager = mServerCore->GetSessionManager();
    sessionManager->RegisterOnSessionConnect(std::bind_front(&ServerFrame::OnPlayerConnect, this));
    sessionManager->RegisterOnSessionDisconnect(std::bind_front(&ServerFrame::OnPlayerDisconnect, this));
}

ServerFrame::~ServerFrame() { 
    mGameScenes.clear();
    mServerCore->End();
}

void ServerFrame::InitGameScenes() {
    mGameScenes.emplace_back(std::make_shared<PlayScene>());

    mCurrentScene = mGameScenes.front();
}

void ServerFrame::GameLoop() {
    while (true) {
        std::this_thread::sleep_for(33ms);

        mTimer->Update();
        const float deltaTime = mTimer->GetDeltaTime();

        mCurrentScene->DispatchPlayerEvent(mPlayerEventQueue);
        mCurrentScene->ProcessPackets(mServerCore, mInputManager);
        mCurrentScene->Update(deltaTime); // Temp
        mCurrentScene->SendUpdateResult(mServerCore);
    }
}

void ServerFrame::OnPlayerConnect(SessionIdType id) {
    auto object = std::make_shared<GameObject>();
    
    object->InitId(id);
    object->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f });
    object->CreateComponent<PlayerScript>(object, mInputManager->GetInput(id));
    object->GetTransform()->Scale(SimpleMath::Vector3{ 10.0f });
    object->SetColor(SimpleMath::Vector3{ RAND_COLOR, RAND_COLOR, RAND_COLOR });

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
