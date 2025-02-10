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

    Lock::SRWLockGuard playerGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, mPlayerLock };
    mPlayers[id] = obj;

    obj->MakeCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f });
    obj->GetTransform()->Scale(SimpleMath::Vector3{ 10.0f });
    obj->SetColor(SimpleMath::Vector3{ RAND_COLOR, RAND_COLOR, RAND_COLOR });

    mCollisionWorld.AddCollisionObject("player", obj);
    mCollisionWorld.AddObjectInTerrainGroup(obj);
    std::cout << std::format("Add player {}\n", id);
}

void PlayScene::ExitPlayer(SessionIdType id) {
    auto it = mPlayers.find(id);
    if (it != mPlayers.end()) {
        Lock::SRWLockGuard playerGuard{ Lock::SRWLockMode::SRW_EXCLUSIVE, mPlayerLock };
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
    Lock::SRWLockGuard playerGuard{ Lock::SRWLockMode::SRW_SHARED, mPlayerLock };
    while (not buffer.IsReadEnd()) {
        buffer.Read(header);
        
        switch (header.type) {
        case PacketType::PT_INPUT_CS:
            {
                PacketInput input;  
                buffer.Read(input);
                
                if (not mPlayers.contains(header.id)) {
                    break;
                }
                mPlayers[input.id]->SetInput(input.key);
            }
            break;

        case PacketType::PT_PLAYER_INFO_CS:
            {
                PacketPlayerInfoCS obj;
                buffer.Read(obj);

                if (not mPlayers.contains(header.id)) {
                    break;
                }
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
    mPlayerLock.ReadLock();
    for (auto& [id, obj] : mPlayers) {
        obj->Update(deltaTime);
    }
    mPlayerLock.ReadUnlock();

    mCollisionWorld.HandleCollision();

    mCollisionWorld.HandleTerrainCollision();
}

void PlayScene::SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) {
    auto sessionManager = serverCore->GetSessionManager();

    PacketPlayerInfoSC playerPacket{ sizeof(PacketPlayerInfoSC), PacketType::PT_PLAYER_INFO_SC };
    mPlayerLock.ReadLock();
    for (auto& [id, player] : mPlayers) {
        playerPacket.id = id;
        playerPacket.color = player->GetColor();
        playerPacket.position = player->GetPosition();
        playerPacket.rotation = player->GetRotation();
        playerPacket.scale = player->GetScale();
        sessionManager->SendAll(&playerPacket);
    }
    mPlayerLock.ReadUnlock();

    PacketGameObject objPacket{ sizeof(PacketGameObject), PacketType::PT_GAME_OBJECT_SC };
    for (size_t id{ 0 }; auto & obj : mObjects) {
        objPacket.objectId = id;
        objPacket.color = obj->GetColor();
        objPacket.position = obj->GetPosition();
        objPacket.rotation = obj->GetRotation();
        objPacket.scale = obj->GetScale();
        sessionManager->SendAll(&objPacket);

        ++id;
    }
}
