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
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

