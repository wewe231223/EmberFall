#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ServerGameScene.h
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
//        02 - 10 : Player를 각자의 게임씬에서 입/퇴장 처리를 하려니 복잡해진다.
//                  차라리 GameFrame에서 플레이어 입/퇴장 처리를 하고 게임 씬에 플레이어 정보를 보내자.
// 
//                  개선:
//                      ServerFrame의 OnPlayerConnect, OnPlayerDisconnect에서 실제 Player의 오브젝트를 생성
//                      PlayerEvent를 발생시키고 concurrent_queue에 저장함.
//                      GameScene에서는 이 concurrent_queue에 저장된 이벤트를 읽어서 Player오브젝트를 추가/삭제
// 
//        02 - 12 : ViewList를 적용하면서 GameScene에서 오브젝트 패킷을 직접 전송하는 코드 삭제
//        02 - 13 : ViewList에서 플레이어 정보도 전송
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "TerrainCollider.h"
#include "GridWorld.h"

class GameObject;

class IServerGameScene abstract : public std::enable_shared_from_this<IServerGameScene> {
public:
    // ServerScene에서 한번에 처리할 수 있는 EVENT (5번의 try_pop)
    static constexpr size_t ONCE_PROCESSING_EVENT = 5;

public:
    IServerGameScene();
    virtual ~IServerGameScene();

public:
    std::vector<std::shared_ptr<GameObject>>& GetPlayers();
    virtual std::vector<std::shared_ptr<GameObject>>& GetObjects() abstract;
    virtual std::shared_ptr<GameObject> GetObjectFromId(NetworkObjectIdType id) abstract;

    virtual void Init() abstract;

    virtual void DispatchPlayerEvent(Concurrency::concurrent_queue<PlayerEvent>& eventQueue);

    virtual void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore, std::shared_ptr<class InputManager>& inputManager) abstract;
    virtual void Update(const float deltaTime) abstract;
    virtual void LateUpdate(const float deltaTime) abstract;

    virtual void AddPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject);
    virtual void ExitPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject);

protected:
    std::vector<std::shared_ptr<GameObject>> mPlayerList{ };
    std::unordered_map<SessionIdType, std::shared_ptr<GameObject>> mPlayers{ };
};

class EchoTestScene : public IServerGameScene {
public:
    EchoTestScene();
    ~EchoTestScene();

public:
    virtual void Init() override { } 

    virtual std::vector<std::shared_ptr<GameObject>>& GetObjects() override { return mObjects; }
    virtual std::shared_ptr<GameObject> GetObjectFromId(NetworkObjectIdType id) override { return nullptr; }

    virtual void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore, std::shared_ptr<class InputManager>& inputManager) override;
    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

private:
    std::vector<std::shared_ptr<GameObject>> mObjects{ };
};

class PlayScene : public IServerGameScene {
    inline static constexpr size_t MAX_OBJECT = 5; // 최대 오브젝트 개수 제한.

public: 
    PlayScene();
    ~PlayScene();

public:
    virtual std::vector<std::shared_ptr<GameObject>>& GetObjects() override;
    virtual std::shared_ptr<GameObject> GetObjectFromId(NetworkObjectIdType id) override;

    virtual void Init() override;

    virtual void ProcessPackets(const std::shared_ptr<ServerCore>& serverCore, std::shared_ptr<class InputManager>& inputManager) override;
    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void AddPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) override;
    virtual void ExitPlayer(SessionIdType id, std::shared_ptr<GameObject> playerObject) override;

private:
    std::vector<std::shared_ptr<GameObject>> mObjects{ };

    std::shared_ptr<class Terrain> mTerrain{ };
    TerrainCollider mTerrainCollider{ };
    GridWorld mGridWorld{ 50.0f, 50.0f, 1000.0f, 1000.0f };
};