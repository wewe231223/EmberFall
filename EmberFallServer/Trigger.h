#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ViewList.h
// 
// 2025 - 03 - 15 : 특정 영역 내에 들어온 오브젝트에게 Event를 보내주는 클래스
//                  GameObject의 Script Component 형태로 구현 -> 03-31 EventTrigger로 변경
// 
//        03 - 17 : 여러 오브젝트가 동시에 영역에 들어와서 공격받는 상황일 때
//                  각각의 공격횟수가 정해져 있다면 결국 몇번 공격받앗는지를 기억할 필요가 있다.
// 
//        03 - 31 : Trigger의 역할을 변경함.
//                  Trigger는 단순하게 범위 내에 들어온 오브젝트를 기록하는 역할만 하게하자.
//                  
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Script.h"

constexpr int32_t EVENT_PRODUCING_ONCE = 1;

enum class TriggerType {
    BASE_TRIGGER,
    EVENT_TRIGGER,
};

class Trigger : public Script {
public:
    Trigger(std::shared_ptr<GameObject> owner, float lifeTime);
    virtual ~Trigger();

public:
    std::unordered_set<NetworkObjectIdType>& GetObjects();

    virtual void Init() override;
    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnCollisionTerrain(const float height) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

protected:
    float mLifeTime{ };
    std::unordered_set<NetworkObjectIdType> mInTriggerObjects{ };
};