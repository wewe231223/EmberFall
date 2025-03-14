#include "pch.h"
#include "ServerGameScene.h"
#include "GameObject.h"
#include "GameEventManager.h"
#include "Terrain.h"
#include "Collider.h"
#include "Input.h"

#include "PlayerScript.h"
#include "MonsterScript.h"
#include "CorruptedGem.h"
#include "ServerFrame.h"

#include "PacketProcessFunctions.h"

IServerGameScene::IServerGameScene() { }

IServerGameScene::~IServerGameScene() { 
    mPacketProcessor.Clear();
}

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
            AddPlayer(event.id, event.player);
            break;

        case PlayerEvent::EventType::DISCONNECT:
            ExitPlayer(event.id);
            break;

        default:
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player Event Error!");
            break;
        }
    }
}

void IServerGameScene::AddPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) {
    mPlayers[id] = playerObject;
    mPlayerList.push_back(playerObject);
}

void IServerGameScene::ExitPlayer(SessionIdType id) {
    auto it = mPlayers.find(id);
    if (it == mPlayers.end()) {
        return;
    }

    auto objSearch = std::find(mPlayerList.begin(), mPlayerList.end(), it->second);
    std::swap(*objSearch, mPlayerList.back());
    mPlayerList.pop_back();

    mPlayers.erase(it);
}

PlayScene::PlayScene() { }

PlayScene::~PlayScene() { }

ObjectList& PlayScene::GetObjects() {
    return mObjects;
}

std::shared_ptr<GameObject> PlayScene::GetObjectFromId(NetworkObjectIdType id) {
    if (id >= OBJECT_ID_START) {
        return mObjects[id];
    }
    else {
        return mPlayers[static_cast<SessionIdType>(id)];
    }
}

void PlayScene::Init() {
    mTerrain = std::make_shared<Terrain>("../Resources/HeightMap.raw");
    mTerrainCollider.SetTerrain(mTerrain);

    mObjects.resize(MAX_OBJECT);
    for (NetworkObjectIdType id{ 0 }; auto & object : mObjects) {
        object = std::make_shared<GameObject>(shared_from_this());
        object->InitId(OBJECT_ID_START + id);
        ++id;

        object->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f });
        object->CreateComponent<MonsterScript>(object);
        object->SetColor(Random::GetRandomColor());
        object->GetTransform()->Translate(Random::GetRandomVec3(-50.0f, 50.0f));
        object->GetTransform()->Scale(SimpleMath::Vector3{ 1.0f });
        object->GetTransform()->SetY(Random::GetRandom<float>(0.0f, 100.0f));

        object->SetActive(true);

        auto& factors = object->GetPhysics()->mFactor;
        factors.mass = 10.0f;

        mTerrainCollider.AddObjectInTerrainGroup(object);

        object->Init();
    }
}

void PlayScene::RegisterPacketProcessFunctions() {
    mPacketProcessor.RegisterProcessFn(PacketType::PACKET_KEYINPUT,
        [=](PacketHeader* header) { ProcessPacketKeyInput(header, gServerFrame->GetInputManager()); });

    mPacketProcessor.RegisterProcessFn(PacketType::PACKET_CAMERA,
        [=](PacketHeader* header) { ProcessPacketCamera(header, mPlayers); });

    mPacketProcessor.RegisterProcessFn(PacketType::PACKET_EXIT,
        [=](PacketHeader* header) { ProcessPacketCamera(header, mPlayers); });

    mPacketProcessor.RegisterProcessFn(PacketType::PACKET_REQUEST_ATTACK,
        [=](PacketHeader* header) { ProcessPacketCamera(header, mPlayers); });

    mPacketProcessor.RegisterProcessFn(PacketType::PACKET_SELECT_ROLE,
        [=](PacketHeader* header) { ProcessPacketCamera(header, mPlayers); });

    mPacketProcessor.RegisterProcessFn(PacketType::PACKET_SELECT_WEAPON,
        [=](PacketHeader* header) { ProcessPacketCamera(header, mPlayers); });
}

void PlayScene::ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) {
    auto packetHandler = serverCore->GetPacketHandler();
    auto& buffer = packetHandler->GetBuffer();
    mPacketProcessor.ProcessPackets(buffer);
}

void PlayScene::Update(const float deltaTime) {
    for (auto& [id, obj] : mPlayers) {
        obj->Update(deltaTime);
    }

    for (auto& object : mObjects) {
        object->Update(deltaTime);
    }

    mTerrainCollider.HandleTerrainCollision();
    mGridWorld.Update(mObjects);

    gEventManager->Update();
}

void PlayScene::LateUpdate(const float deltaTime) { 
    for (auto& [id, obj] : mPlayers) {
        obj->LateUpdate(deltaTime);
    }

    for (auto& object : mObjects) {
        object->LateUpdate(deltaTime);
    }
}

void PlayScene::AddPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) {
    mPlayers[id] = playerObject;
    mPlayerList.push_back(playerObject);

    playerObject->GetComponent<PlayerScript>()->ResetGameScene(shared_from_this());
    playerObject->GetTransform()->Translate(Random::GetRandomVec3(-100.0f, 100.0f));

    mTerrainCollider.AddObjectInTerrainGroup(playerObject);

    playerObject->Init();
}

void PlayScene::ExitPlayer(SessionIdType id) {
    auto it = mPlayers.find(id);
    if (it == mPlayers.end()) {
        return;
    }

    auto objSearch = std::find(mPlayerList.begin(), mPlayerList.end(), it->second);
    std::swap(*objSearch, mPlayerList.back());
    mPlayerList.pop_back();

    mTerrainCollider.RemoveObjectFromTerrainGroup(it->second);
    mPlayers.erase(it);
}
