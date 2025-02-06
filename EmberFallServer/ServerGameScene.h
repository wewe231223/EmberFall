#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ServerGameScene.cpp
// 
// 2025 - 02 - 02 : GameScene에서 SessionManager의 Connect, Disconnect 가 일어날 때 호출될 함수를 등록하고
//                  그 함수안에서 GameObject의 추가/삭제를 하는 점이 별로인거 같다.
//                  어짜피 추가/삭제할 때마다 locking은 해주어야 하고, 이미 Session을 추가/ 삭제할 때 locking을 하고 있어서
//                  lock을 여러번 거는 것도 문제점인거 같다.
// 
//                  -> 그냥 GameObject가 Session을 상속하게 하는건?
//                      -> Session을 상속했을 때 얻는 이점이 별로 없는거 같다.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CollisionWorld.h"

class IServerGameScene abstract {
public:
    IServerGameScene();
    virtual ~IServerGameScene();

public:
    virtual void RegisterOnSessionConnect(const std::shared_ptr<ServerCore>& serverCore) abstract;
    virtual void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) abstract;
    virtual void Update(const float deltaTime) abstract;
    virtual void SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) abstract;
};

class EchoTestScene : public IServerGameScene {
public:
    EchoTestScene();
    ~EchoTestScene();

public:
    virtual void RegisterOnSessionConnect(const std::shared_ptr<ServerCore>& serverCore) override;
    void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) override;
    virtual void Update(const float deltaTime) override;
    virtual void SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) override;
};

class PlayScene : public IServerGameScene {
public:
    PlayScene();
    ~PlayScene();

public:
    virtual void RegisterOnSessionConnect(const std::shared_ptr<ServerCore>& serverCore) override;
    void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) override;
    virtual void Update(const float deltaTime) override;
    virtual void SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) override;

public:
    void EnterPlayer(SessionIdType id);
    void ExitPlayer(SessionIdType id);

private:
    std::mutex mGameObjectLock{ };
    std::unordered_map<SessionIdType, std::shared_ptr<class GameObject>> mPlayers{ };

    std::shared_ptr<class Terrain> mTerrain{ }; // test
    CollisionWorld mCollisionWorld{ };
};