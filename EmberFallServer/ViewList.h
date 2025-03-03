#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ViewList.h
// 
// 2025 - 02 - 12 : 패킷양을 줄이기 위한 ViewList
//                  매번 업데이트마다 시야 범위 내에 오브젝트가 있는지 검사하고  정보를 보낸다.
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <set>

class ViewList {
public:
    ViewList(SessionIdType ownerId, std::shared_ptr<SessionManager> sessionManager);
    ~ViewList();

    ViewList& operator=(const ViewList& other);
    ViewList& operator=(ViewList&& other) noexcept;

public:
    void Update();
    void Send();

    void AddInRange(std::shared_ptr<class GameObject> obj);
    bool EraseFromRange(std::shared_ptr<class GameObject> obj);

    std::set<std::shared_ptr<class GameObject>>& GetInRangeObjects();

public:
    GameUnits::GameUnit<GameUnits::Meter> mViewRange{ 100.0m };
    SimpleMath::Vector3 mPosition{ };
    std::shared_ptr<class IServerGameScene> mCurrentScene{ };

private:
    SessionIdType mOwnerId{ };
    std::shared_ptr<SessionManager> mSessionManager{ };
    std::set<std::shared_ptr<class GameObject>> mObjectInRange{ };
};