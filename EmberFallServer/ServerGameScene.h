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
//                  -> 어짜피 Player에 대한 정보를 보낼 때, 처리 할 때에는 Locking을 해야한다.
//                      -> 그냥 Lock을 걸자...
//                     
// 
//        02 - 09 : Object 동기화에 대한 고민
//                  모든 Object가 자신의 고유한 Id를 가지게 해야 서버-클라이언트간 동기화가 가능하다.
//                  그게 아니라면 게임 씬 자체를 리플리케이션하고 그 데이터를 전송하는 방법밖에는 없다.
// 
//                  일단 오브젝트는 미리 다 만들어 놓고 쓰는걸로 하고, 고정 Array를 쓰는걸로 하자.
//                  Id는 일단 배열의 index로 설정을 하자.
//                  
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
    virtual void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) override;
    virtual void Update(const float deltaTime) override;
    virtual void SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) override;
};

class PlayScene : public IServerGameScene {
    inline static constexpr size_t MAX_OBJECT = 5000; // 최대 오브젝트 개수 제한.

public:
    PlayScene();
    ~PlayScene();

public:
    virtual void RegisterOnSessionConnect(const std::shared_ptr<ServerCore>& serverCore) override;
    virtual void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore) override;
    virtual void Update(const float deltaTime) override;
    virtual void SendUpdateResult(const std::shared_ptr<ServerCore>& serverCore) override;

public:
    void EnterPlayer(SessionIdType id);
    void ExitPlayer(SessionIdType id);

private:
    Lock::SRWLock mPlayerLock{ };
    std::unordered_map<SessionIdType, std::shared_ptr<class GameObject>> mPlayers{ };
    std::vector<std::shared_ptr<class GameObject>> mObjects{ };

    std::shared_ptr<class Terrain> mTerrain{ }; // test
    CollisionWorld mCollisionWorld{ };
};