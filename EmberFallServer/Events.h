#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Event.h
// 
// 2025 - 02 - 10 : 각종 이벤트 타입들을 정리한 헤더. pch에 포함.
// 
//      PlayerEvent -> Player (Session) 접속/퇴장시 발생하는 이벤트
//                      접속/퇴장시 해야하는 객체 삭제/추가를 ServerFrame에서 담당하고 GameScene에 
//                      해당 플레이어의 접속/퇴장 정보를 줌.
// 
//        02 - 13 : GameEvent 작성 중... 어떻게 처리할 지도 생각해보자.
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

//struct TimerEvent {
//    using EventFn = std::function<bool(const float)>;
//
//    float delay;
//    float timeCount;
//    EventFn fn;
//};

struct PlayerEvent {
    enum class EventType {
        CONNECT,
        DISCONNECT,
        NONE
    };

    EventType eventType;
    SessionIdType id;
    std::shared_ptr<class GameObject> player;
};

struct CollisionEvent {
    std::shared_ptr<class GameObject> object1;
    std::shared_ptr<class GameObject> object2;
    SimpleMath::Vector3 transVector;
};

struct CollisionContact {
    std::shared_ptr<GameObject> objectA;
    std::shared_ptr<GameObject> objectB;
    SimpleMath::Vector3 minTrans;          // 침투 벡터
    SimpleMath::Vector3 contactNormal;     // 충돌 법선 벡터
    float penetrationDepth;                // 침투 깊이
    SimpleMath::Vector3 accumulatedImpulse; // 이전 프레임의 임펄스 (Contact Warming)
};

struct GameEvent {
    NetworkObjectIdType receiver;
    NetworkObjectIdType sender;
};

struct AttackEvent : public GameEvent { 
    float damage;
    float duration;
    float attackDelay;
};

struct EventBus {
public:
    enum class EventType {
        PLAYER_EVENT,
        GAME_EVENT,
    };
};