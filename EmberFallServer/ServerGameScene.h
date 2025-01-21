#pragma once

class IServerGameScene abstract {
public:
    IServerGameScene();
    virtual ~IServerGameScene();

public:
    virtual void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) abstract;
    virtual void Update(const float deltaTime) abstract;
};

class EchoTestScene : public IServerGameScene {
public:
    EchoTestScene();
    ~EchoTestScene();

public:
    void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) override;
    virtual void Update(const float deltaTime) override;
};

class PlayScene : IServerGameScene {
public:
    PlayScene();
    ~PlayScene();

public:
    void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) override;
    virtual void Update(const float deltaTime) override;
};