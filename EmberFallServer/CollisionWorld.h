#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CollisionWorld.h
// 
// 2025 - 02 - 05 김성준 : 충돌처리를 담당할 클래스
//                      게임 월드에서 실제 충돌처리를 진행할 물리 세계를 새롭게 만듦
//                      Observer 패턴에서 영감을 얻음
// 
//                      GameObject 가 OnHandleCollision이라는 함수를 가지고 있어야하고,
//                      충돌이 감지되면 CollisionEvent를 새로 생성하여 각각의 오브젝트에게 이 이벤트를 전달함.
// 
//                      여러개의 물리 세계를 생성하는 것은 제한하지 않으나 복사, 이동은 제한
// 
//                      충돌 처리는 각 그룹을 순회하면서 그룹 내의 모든 오브젝트에 대해 충돌처리를 수행함
//                      
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GameObject;

using CollisionList = std::vector<std::shared_ptr<GameObject>>;
using CollisionPair = std::pair<CollisionList, CollisionList>;
using CollisionMap = std::unordered_map<std::string, CollisionPair>;
using GroupMap = std::vector<std::string>;

class CollisionWorld {
public:
    CollisionWorld();
    ~CollisionWorld();

    CollisionWorld(const CollisionWorld&) = delete;
    CollisionWorld(CollisionWorld&&) noexcept = delete;
    CollisionWorld& operator=(const CollisionWorld&) = delete;
    CollisionWorld& operator=(CollisionWorld&&) noexcept = delete;

public:
    bool Contains(const std::string& groupTag) const;
    // 서로 다른 두 오브젝트 간의 충돌체크 그룹 설정 (ex: player-enemy) 같은 오브젝트 간의 충돌체크 그룹 설정은 AddCollisionObject
    void AddCollisionPair(const std::string& groupTag, std::shared_ptr<GameObject> obj1=nullptr, std::shared_ptr<GameObject> obj2=nullptr);
    // 같은 오브젝트 간의 충돌체크 그룹 설정 (ex: player-player)
    void AddCollisionObject(const std::string& groupTag, std::shared_ptr<GameObject> obj);
    void RemoveObejctFromGroup(const std::string& groupTag, std::shared_ptr<GameObject> obj);
    void HandleCollision();

private:
    void HandleCollision(const std::string& groupTag, std::shared_ptr<GameObject>& obj1, std::shared_ptr<GameObject>& obj2);
    void HandleCollisionListPair(const std::string& groupTag, CollisionPair& listPair);
    void HandleCollisionList(const std::string& groupTag, CollisionList& list);

private:
    CollisionMap mCollisionWorld{ };
};

