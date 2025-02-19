#include "pch.h"
#include "ServerGameScene.h"
#include "GameObject.h"
#include "Terrain.h"
#include "Collider.h"
#include "Input.h"
#include "PlayerScript.h"
#include "MonsterScript.h"

IServerGameScene::IServerGameScene() { }

IServerGameScene::~IServerGameScene() { }

std::vector<std::shared_ptr<GameObject>>& IServerGameScene::GetPlayers() {
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
            ExitPlayer(event.id, event.player);
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

void IServerGameScene::ExitPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) {
    auto it = mPlayers.find(id);
    if (it == mPlayers.end()) {
        return;
    }

    auto objSearch = std::find(mPlayerList.begin(), mPlayerList.end(), playerObject);
    std::swap(*objSearch, mPlayerList.back());
    mPlayerList.pop_back();

    mPlayers.erase(it);
}

EchoTestScene::EchoTestScene() { }

EchoTestScene::~EchoTestScene() { }

void EchoTestScene::ProcessPackets(const std::shared_ptr<ServerCore>& serverCore, std::shared_ptr<InputManager>& inputManager) {
    auto packetHandler = serverCore->GetPacketHandler();
    auto& buffer = packetHandler->GetBuffer();
    if (0 == buffer.Size()) {
        return;
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Total RecvBytes: {}", buffer.Size());
    PacketChat chat{ };
    while (not buffer.IsReadEnd()) {
        buffer.Read(chat);
        chat.chatdata[chat.size] = '\0';
        std::cout << chat.chatdata << std::endl;
    }

    auto sessionManager = serverCore->GetSessionManager();
    sessionManager->SendAll(buffer.Data(), buffer.Size());
}

void EchoTestScene::Update(const float deltaTime) { }

void EchoTestScene::LateUpdate(const float deltaTime) { }

PlayScene::PlayScene() {
    mTerrain = std::make_shared<Terrain>("../Resources/HeightMap.raw");
    mCollisionWorld.AddTerrain(mTerrain);

    mObjects.resize(MAX_OBJECT);
    for (size_t id{ 0 }; auto & object : mObjects) {
        object = std::make_shared<GameObject>();
        object->InitId(OBJECT_ID_START + id);
        ++id;

        object->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f });
        object->CreateComponent<MonsterScript>(object);
        object->SetColor(Random::GetRandomColor());
        object->GetTransform()->Translate(Random::GetRandomVec3(-500.0f, 500.0f));
        object->GetTransform()->Scale(SimpleMath::Vector3{ 10.0f });
        object->GetTransform()->SetY(Random::GetRandom<float>(0.0f, 100.0f));

        object->SetActive(true);

        auto& factors = object->GetPhysics()->mFactor;
        factors.mass = 10.0f;

        mCollisionWorld.AddCollisionPair("Player-Object", nullptr, object);
        mCollisionWorld.AddCollisionObject("Object", object);
        mCollisionWorld.AddObjectInTerrainGroup(object);
    }
}

PlayScene::~PlayScene() { }

std::vector<std::shared_ptr<class GameObject>>& PlayScene::GetObjects() {
    return mObjects;
}

void PlayScene::ProcessPackets(const std::shared_ptr<ServerCore>& serverCore, std::shared_ptr<InputManager>& inputManager) {
    auto packetHandler = serverCore->GetPacketHandler();
    auto& buffer = packetHandler->GetBuffer();
    if (0 == buffer.Size()) {
        return;
    }

    PacketHeader header{ };
    while (not buffer.IsReadEnd()) {
        buffer.Read(header);
        if (not mPlayers.contains(header.id)) {
            buffer.AdvanceReadPos(header.size);
            break;
        }
        
        switch (header.type) {
        case PacketType::PT_INPUT_CS:
            {
                PacketInput inputPacket;  
                buffer.Read(inputPacket);
                auto input = inputManager->GetInput(inputPacket.id);
                input->UpdateInput(inputPacket.key);
            }
            break;

        case PacketType::PT_PLAYER_INFO_CS:
            {
                PacketPlayerInfoCS obj;
                buffer.Read(obj);
                mPlayers[obj.id]->GetTransform()->Rotation(obj.rotation);
            }
        break;

        default:
            gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "PacketError Size: {}, Type: {}", header.size, header.type);
            break;
        }
    }
}

void PlayScene::Update(const float deltaTime) {
    for (auto& [id, obj] : mPlayers) {
        obj->Update(deltaTime);
    }

    for (auto& object : mObjects) {
        object->Update(deltaTime);
    }

    mGridWorld.Update(mObjects);

    mCollisionWorld.HandleTerrainCollision();
    //mCollisionWorld.HandleCollision();
}

void PlayScene::LateUpdate(const float deltaTime) { 
}

void PlayScene::AddPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) {
    mPlayers[id] = playerObject;
    mPlayerList.push_back(playerObject);

    playerObject->GetComponent<PlayerScript>()->ResetGameScene(shared_from_this());
    playerObject->GetTransform()->Translate(Random::GetRandomVec3(-100.0f, 100.0f));

    mCollisionWorld.AddCollisionObject("Player", playerObject);
    mCollisionWorld.AddCollisionPair("Player-Object", playerObject);
    mCollisionWorld.AddObjectInTerrainGroup(playerObject);
}

void PlayScene::ExitPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) {
    auto it = mPlayers.find(id);
    if (it == mPlayers.end()) {
        return;
    }

    auto objSearch = std::find(mPlayerList.begin(), mPlayerList.end(), playerObject);
    std::swap(*objSearch, mPlayerList.back());
    mPlayerList.pop_back();

    mCollisionWorld.RemoveObjectFromGroup("Player", playerObject);
    mCollisionWorld.RemoveObjectFromGroup("Player-Object", playerObject);
    mCollisionWorld.RemoveObjectFromTerrainGroup(playerObject);

    mPlayers.erase(it);
}
