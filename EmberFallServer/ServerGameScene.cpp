#include "pch.h"
#include "ServerGameScene.h"

IServerGameScene::IServerGameScene() { }

IServerGameScene::~IServerGameScene() { }

EchoTestScene::EchoTestScene() { }

EchoTestScene::~EchoTestScene() { }

void EchoTestScene::ProcessPackets() {
    auto& buffer = gPacketHandler->GetBuffer();
    if (0 == buffer.Size()) {
        return;
    }

    std::cout << "Total RecvBytes: " << buffer.Size() << std::endl;
    std::cout << "Contents: " << buffer.Data() << std::endl;

    gSessionManager->SendAll(buffer.Data(), buffer.Size());
}

void EchoTestScene::Update(const float deltaTime) {
}
