#include "pch.h"
#include "ServerGameScene.h"
#include "GameObject.h"
#include "Terrain.h"
#include "Collider.h"
#include "Input.h"

IServerGameScene::IServerGameScene() { }

IServerGameScene::~IServerGameScene() { }

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
}

void IServerGameScene::ExitPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) {
    auto it = mPlayers.find(id);
    if (it == mPlayers.end()) {
        return;
    }

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

void EchoTestScene::SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) { }

PlayScene::PlayScene() {
    mTerrain = std::make_shared<Terrain>("../Resources/HeightMap.raw");
    mCollisionWorld.AddTerrain(mTerrain);
}

PlayScene::~PlayScene() { }

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

    mCollisionWorld.HandleCollision();
    mCollisionWorld.HandleTerrainCollision();
}

void PlayScene::SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) {
    auto sessionManager = serverCore->GetSessionManager();

    PacketPlayerInfoSC playerPacket{ sizeof(PacketPlayerInfoSC), PacketType::PT_PLAYER_INFO_SC };
    for (auto& [id, player] : mPlayers) {
        playerPacket.id = id;
        playerPacket.color = player->GetColor();
        playerPacket.position = player->GetPosition();
        playerPacket.rotation = player->GetRotation();
        playerPacket.scale = player->GetScale();
        sessionManager->SendAll(&playerPacket);
    }

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

void PlayScene::AddPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) {
    mPlayers[id] = playerObject;
    mCollisionWorld.AddCollisionObject("Player", playerObject);
    mCollisionWorld.AddObjectInTerrainGroup(playerObject);
}

void PlayScene::ExitPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) {
    auto it = mPlayers.find(id);
    if (it == mPlayers.end()) {
        return;
    }

    mCollisionWorld.RemoveObjectFromGroup("Player", playerObject);
    mCollisionWorld.RemoveObjectFromTerrainGroup(playerObject);

    mPlayers.erase(it);
}
