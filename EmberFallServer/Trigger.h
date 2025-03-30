#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ViewList.h
// 
// 2025 - 03 - 15 : 특정 영역 내에 들어온 오브젝트에게 Event를 보내주는 클래스
//                  GameObject의 Script Component 형태로 구현
// 
//        03 - 17 : 여러 오브젝트가 동시에 영역에 들어와서 공격받는 상황일 때
//                  각각의 공격횟수가 정해져 있다면 결국 몇번 공격받앗는지를 기억할 필요가 있다.
//                  
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Script.h"

constexpr int32_t EVENT_PRODUCING_ONCE = 1;

class Trigger : public Script {
public:
    Trigger(std::shared_ptr<GameObject> owner, std::shared_ptr<GameEvent> event,
        float lifeTime, float eventDelay, int32_t eventCount);
    virtual ~Trigger();

public:
    virtual void Init() override;
    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;
    virtual void OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override final;
    virtual void OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override final;
    virtual void OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override final;

    virtual void OnCollisionTerrain(const float height) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

private:
    float mLifeTime{ };
    float mProduceEventDelay{ };

    int32_t mProduceEventCount{ };

    std::shared_ptr<GameEvent> mEvent{ };
    std::unordered_map<NetworkObjectIdType, std::pair<float, int32_t>> mProducedEventCounter{ };
};