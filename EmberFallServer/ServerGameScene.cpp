#include "pch.h"
#include "ServerGameScene.h"
#include "GameObject.h"
#include "Terrain.h"
#include "Collider.h"

IServerGameScene::IServerGameScene() { }

IServerGameScene::~IServerGameScene() { }

EchoTestScene::EchoTestScene() { }

EchoTestScene::~EchoTestScene() { }

void EchoTestScene::RegisterOnSessionConnect(const std::shared_ptr<ServerCore>& serverCore) { }

void EchoTestScene::ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) {
    auto packetHandler = serverCore->GetPacketHandler();
    auto& buffer = packetHandler->GetBuffer();
    if (0 == buffer.Size()) {
        return;
    }

    std::cout << "Total RecvBytes: " << buffer.Size() << std::endl;
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

void EchoTestScene::SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) { }

PlayScene::PlayScene() {
    mTerrain = std::make_shared<Terrain>("../Resources/HeightMap.raw");
    mCollisionWorld.AddTerrain(mTerrain);
}

PlayScene::~PlayScene() { }

#define RAND_COLOR static_cast<float>(rand()) / static_cast<float>(RAND_MAX) // TEST

void PlayScene::EnterPlayer(SessionIdType id) {
    std::shared_ptr<GameObject> obj = std::make_shared<GameObject>();
    obj->InitId(id);
    mPlayers[id] = obj;
    obj->MakePhysics();
    obj->MakeCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f });
    obj->SetColor(SimpleMath::Vector3{ RAND_COLOR, RAND_COLOR, RAND_COLOR });
    mCollisionWorld.AddCollisionObject("player", obj);
    mCollisionWorld.AddObjectInTerrainGroup(obj);
    std::cout << std::format("Add player {}\n", id);
}

void PlayScene::ExitPlayer(SessionIdType id) {
    auto it = mPlayers.find(id);
    if (it != mPlayers.end()) {
        std::cout << std::format("Erase player {}\n", id);
        mCollisionWorld.RemoveObjectFromGroup("player", it->second);
        mCollisionWorld.RemoveObjectFromTerrainGroup(it->second);
        mPlayers.erase(it);
    }
}

void PlayScene::RegisterOnSessionConnect(const std::shared_ptr<ServerCore>& serverCore) {
    auto sessionManager = serverCore->GetSessionManager();
    sessionManager->RegisterOnSessionConnect(std::bind_front(&PlayScene::EnterPlayer, this));
    sessionManager->RegisterOnSessionDisconnect(std::bind_front(&PlayScene::ExitPlayer, this));
}

void PlayScene::ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) {
    auto packetHandler = serverCore->GetPacketHandler();
    auto& buffer = packetHandler->GetBuffer();
    if (0 == buffer.Size()) {
        return;
    }

    PacketHeader header{ };
    while (not buffer.IsReadEnd()) {
        buffer.Read(header);
        
        switch (header.type) {
        case PacketType::PT_INPUT_CS:
            {
                PacketInput input;  
                buffer.Read(input);
                mPlayers[input.id]->SetInput(input.key);
            }
            break;

        case PacketType::PT_GAMEOBJ_CS:
            {
                PacketGameObjCS obj;
                buffer.Read(obj);
                mPlayers[obj.id]->GetTransform()->Rotation(obj.rotation);
            }
        break;

        default:
            std::cout << std::format("PacketError Size: {}, Type: {}\n", header.size, header.type);
            break;
        }
    }
}

void PlayScene::Update(const float deltaTime) { 
    for (auto& [id, obj] : mPlayers) {
        obj->UpdateInput(deltaTime);
    }

    for (auto& [id, obj] : mPlayers) {
        obj->Update(deltaTime);
    }

    mCollisionWorld.HandleCollision();

    mCollisionWorld.HandleTerrainCollision();
}

void PlayScene::SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) {
    auto sessionManager = serverCore->GetSessionManager();
    PacketGameObj objPacket{ sizeof(PacketGameObj), PacketType::PT_GAMEOBJ_SC, 0 };
    for (auto& [id, obj] : mPlayers) { // lock 은 안걸고 일단 보내보자 어짜피 connect 상태가 아니라면 send는 실패할것
        objPacket.id = id;
        objPacket.color = obj->GetColor();
        objPacket.position = obj->GetPosition();
        objPacket.rotation = obj->GetRotation();
        objPacket.scale = obj->GetScale();
        sessionManager->SendAll(&objPacket);
    }
}
