#include "pch.h"
#include "ServerGameScene.h"

IServerGameScene::IServerGameScene() { }

IServerGameScene::~IServerGameScene() { }

EchoTestScene::EchoTestScene() { }

EchoTestScene::~EchoTestScene() { }

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

void EchoTestScene::Update(const float deltaTime) {
}

PlayScene::PlayScene() { }

PlayScene::~PlayScene() { }

void PlayScene::ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) { 
    auto packetHandler = serverCore->GetPacketHandler();
    auto& buffer = packetHandler->GetBuffer();
    if (0 == buffer.Size()) {
        return;
    }
}

void PlayScene::Update(const float deltaTime) { }
