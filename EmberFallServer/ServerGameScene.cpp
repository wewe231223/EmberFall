#include "pch.h"
#include "ServerGameScene.h"
#include "GameObject.h"

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

PlayScene::PlayScene() { }

PlayScene::~PlayScene() { }

void PlayScene::RegisterOnSessionConnect(const std::shared_ptr<ServerCore>& serverCore) {
    auto sessionManager = serverCore->GetSessionManager();

    sessionManager->RegisterOnSessionConnect([=](SessionIdType id) {
        std::lock_guard objectGuard{ mGameObjectLock };
        GameObject obj{ };
        obj.InitId(id);
        mPlayers[id] = obj;
        std::cout << std::format("Add player {}\n", id);
    });

    sessionManager->RegisterOnSessionDisconnect([=](SessionIdType id) {
        std::lock_guard objectGuard{ mGameObjectLock };
        auto it = mPlayers.find(id);
        if (it != mPlayers.end()) {
            std::cout << std::format("Erase player {}\n", id);
            mPlayers.erase(it);
        }
    });
}

void PlayScene::ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) {
    auto packetHandler = serverCore->GetPacketHandler();
    auto& buffer = packetHandler->GetBuffer();
    if (0 == buffer.Size()) {
        return;
    }

    PacketInput input{ };
    while (not buffer.IsReadEnd()) {
        buffer.Read(input);
        
        mPlayers[input.id].SetInput(input.key);
    }
}

void PlayScene::Update(const float deltaTime) { 
    for (auto& [id, obj] : mPlayers) {
        obj.Update(deltaTime);
    }
}

void PlayScene::SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) {
    auto sessionManager = serverCore->GetSessionManager();
    PacketGameObj objPacket{ sizeof(PacketGameObj), PacketType::PT_GAMEOBJ_SC, 0 };
    for (auto& [id, obj] : mPlayers) { // lock 은 안걸고 일단 보내보자 어짜피 connect 상태가 아니라면 send는 실패할것
        objPacket.id = id;
        auto objWorld = obj.GetWorld(); 
        std::memcpy(&objPacket.world, &objWorld, sizeof(SimpleMath::Matrix));
        sessionManager->SendAll(&objPacket);
    }
}
