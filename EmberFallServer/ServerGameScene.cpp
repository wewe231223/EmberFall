#include "pch.h"
#include "ServerGameScene.h"
#include "FbsPacketProcessFn.h"
#include "GameEventManager.h"
#include "Terrain.h"
#include "Input.h"
#include "PlayerScript.h"
#include "ServerFrame.h"
#include "ObjectSpawner.h"

#include <profileapi.h>

#include "GameTimer.h"

IServerGameScene::IServerGameScene() { }

IServerGameScene::~IServerGameScene() { }

PlayerList& IServerGameScene::GetPlayers() {
    return mPlayerList;
}

void IServerGameScene::DispatchPlayerEvent(Concurrency::concurrent_queue<PlayerEvent>& eventQueue) {
    for (size_t i = 0; i < ONCE_PROCESSING_EVENT; ++i) {
        PlayerEvent event{ };
        if (false == eventQueue.try_pop(event)) {
            return;
        }

        switch (event.eventType) {
        case PlayerEvent::EventType::CONNECT:
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "AddPlayer");
            AddPlayer(event.id, event.player);
            break;

        case PlayerEvent::EventType::DISCONNECT:
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player Exit");
            ExitPlayer(event.id);
            break;

        default:
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player Event Error!");
            break;
        }
    }
}

void IServerGameScene::AddPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) {
    auto player = playerObject; // std::shared_ptr 복사
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "player use count: {}", player.use_count());
    mPlayers.try_emplace(id, player);
    mPlayerList.push_back(player);
}

void IServerGameScene::ExitPlayer(SessionIdType id) {
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Fuck");

    auto it = mPlayers.find(id);
    if (it == mPlayers.end()) {
        return;
    }

    auto objSearch = std::find(mPlayerList.begin(), mPlayerList.end(), it->second);
    std::swap(*objSearch, mPlayerList.back());
    mPlayerList.pop_back();

    mPlayers.erase(it);

    decltype(auto) packetExit = FbsPacketFactory::PlayerExitSC(id);
    gServerCore->SendAll(packetExit);
}

PlayScene::PlayScene() { }

PlayScene::~PlayScene() { }

ObjectList& PlayScene::GetObjects() {
    return mObjects;
}

std::shared_ptr<GameObject> PlayScene::GetObjectFromId(NetworkObjectIdType id) {
    if (id >= OBJECT_ID_START) {
        return mObjects.at(id - OBJECT_ID_START);
    }
    else if (mPlayers.contains(id)) {
        return mPlayers[static_cast<SessionIdType>(id)];
    }
}

std::shared_ptr<GameObject>& PlayScene::GetInvalidObject() {
    return *std::find_if(mObjects.begin(), mObjects.end(),
        [](const std::shared_ptr<GameObject>& obj) { return not obj->mSpec.active; });
}

TerrainCollider& PlayScene::GetTerrainCollider() {
    return mTerrainCollider;
}

void PlayScene::Init() {
    mTerrain = std::make_shared<Terrain>("../Resources/Binarys/Terrain/TerrainBaked.bin");
    mTerrainCollider.SetTerrain(mTerrain);

    mObjects.resize(MAX_OBJECT);
    for (NetworkObjectIdType id{ 0 }; auto & object : mObjects) {
        object = std::make_shared<GameObject>(shared_from_this());
        object->InitId(OBJECT_ID_START + id);
        ++id;

        object->mSpec.active = false;

        auto& factors = object->GetPhysics()->mFactor;
        factors.mass = 10.0f;
    }

    for (int32_t i = 0; i < 10; ++i) {
        gObjectSpawner->SpawnObject(ObjectTag::MONSTER);
        if (0 == i) {
            decltype(auto) obj = gObjectSpawner->SpawnObject(ObjectTag::CORRUPTED_GEM);
        }
    }
}

void PlayScene::Update(const float deltaTime) {
    auto packetHandler = gServerCore->GetPacketHandler();
    auto& buffer = packetHandler->GetBuffer();

    decltype(auto) sharedThis = shared_from_this();
    decltype(auto) dataPtr = reinterpret_cast<const uint8_t*>(buffer.Data());
    ProcessPackets(sharedThis, dataPtr, buffer.Size());

    StaticTimer::PushTimerEvent(
        []()  {
            unsigned long long time;
            ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));
            gServerCore->SendAll(FbsPacketFactory::PacketLatencySC(time));
        }, 
        0.0f, 
        std::numeric_limits<int32_t>::max()
    );

    for (auto& [id, obj] : mPlayers) {
        obj->Update(deltaTime);
    }

    for (auto& object : mObjects) {
        object->Update(deltaTime);
    }

    mTerrainCollider.HandleTerrainCollision();
    mGridWorld.Update(shared_from_this());

    gEventManager->Update();
}

void PlayScene::LateUpdate(const float deltaTime) { 
    std::erase_if(mPlayerList, [=](const std::shared_ptr<GameObject>& obj) { return not obj->mSpec.active; });

    for (auto& [id, obj] : mPlayers) {
        obj->LateUpdate(deltaTime);
    }

    for (auto& object : mObjects) {
        object->LateUpdate(deltaTime);
    }
}

void PlayScene::AddPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) {
    auto player = playerObject; // std::shared_ptr 복사

    mPlayers.try_emplace(id, player);
    mPlayerList.push_back(player);

    player->GetComponent<PlayerScript>()->ResetGameScene(shared_from_this());
    auto randPos = Random::GetRandomVec3(-50.0f, 50.0f);
    randPos.y = 0.0f;

    player->GetTransform()->Translate(randPos);

    mTerrainCollider.AddObjectInTerrainGroup(player);

    player->Init();
}

void PlayScene::ExitPlayer(SessionIdType id) {
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Exit Player: {}", id);

    auto it = mPlayers.find(id);
    if (it == mPlayers.end()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Exit Player Find Fail: {}", id);
        return;
    }

    it->second->mSpec.active = false;

    auto objSearch = std::find(mPlayerList.begin(), mPlayerList.end(), it->second);
    std::swap(*objSearch, mPlayerList.back());
    mPlayerList.pop_back();

    mTerrainCollider.RemoveObjectFromTerrainGroup(it->second);
    mPlayers.erase(id);

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Call Player Exit");

    decltype(auto) packetExit = FbsPacketFactory::PlayerExitSC(id);
    gServerCore->SendAll(packetExit);
}