#pragma once

class IServerGameScene abstract {
public:
    IServerGameScene();
    virtual ~IServerGameScene();

public:
    virtual void ProcessPackets() abstract;
    virtual void Update(const float deltaTime) abstract;
};

class EchoTestScene : public IServerGameScene {
public:
    EchoTestScene();
    ~EchoTestScene();

public:
    void ProcessPackets() override;
    virtual void Update(const float deltaTime) override;
};