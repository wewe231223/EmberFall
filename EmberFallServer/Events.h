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
//        02 - 21 : GameEvent 처리는 Object에 위임하고 EventManager를 만들어 놓자.
//                  전역 변수를 쓰기는 싫었지만 이경우에는 써야할거 같다...
// 
//        02 - 27 : GameEvent를 매번 모든 오브젝트에게 보내는 건 너무 낭비가 심하다.
//                  차라리 Sender, Receiver를 같이 기억하게 하고 Receiver만 처리하게 하자
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class GameEventType : uint16_t {
    ATTACK_EVENT,
    DESTROY_GEM_EVENT,
    DESTROY_GEM_COMPLETE,
};

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

struct GameEvent {
    GameEventType type;
    NetworkObjectIdType sender;
    NetworkObjectIdType receiver;

    GameEvent() = default;
    GameEvent(GameEventType type, NetworkObjectIdType sender, NetworkObjectIdType receiver)
        : type(type), sender(sender), receiver(receiver) { }
};

struct AttackEvent : public GameEvent {
    float damage;

    AttackEvent() = default;
    AttackEvent(GameEventType type, NetworkObjectIdType sender, NetworkObjectIdType receiver, float damage)
        : GameEvent{ type, sender, receiver }, damage{ damage } { }
};

struct GemDestroyStart : public GameEvent {
    float holdTime; // 키 입력 유지시간.

    GemDestroyStart() = default;
    GemDestroyStart(GameEventType type, NetworkObjectIdType sender, NetworkObjectIdType receiver, float holdTime)
        : GameEvent{ type, sender, receiver }, holdTime{ holdTime } { }
};

struct GemDestroyed : public GameEvent { 
    using GameEvent::GameEvent;
};

// 초기화를 편하게 하기 위해 만든 함수
template <typename EventT> requires std::derived_from<EventT, GameEvent>
inline constexpr auto ConvertEventTypeToEnum() noexcept
{
    // Client To Server
    if constexpr (std::is_same_v<EventT, AttackEvent>) {
        return GameEventType::ATTACK_EVENT;
    }
    else if constexpr (std::is_same_v<EventT, GemDestroyStart>) {
        return GameEventType::DESTROY_GEM_EVENT;
    }
    else if constexpr (std::is_same_v<EventT, GemDestroyed>) {
        return GameEventType::DESTROY_GEM_COMPLETE;
    }
    else {
        static_assert(false, "Packet Type is not exists!!!");
    }
}