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
////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class GameEventType : UINT16 {
    ATTACK_EVENT,
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
};

struct AttackEvent : public GameEvent {
    float damage;
};