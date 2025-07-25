#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constant.h
// 2025 - 01 - 14 (설명 추가 날짜)
//      김성준: 여러 상수, 전역 객체 선언
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline constexpr SessionIdType INVALID_SESSION_ID = std::numeric_limits<SessionIdType>::max();
inline constexpr NetworkObjectIdType INVALID_OBJ_ID = std::numeric_limits<NetworkObjectIdType>::max();

inline constexpr size_t MAX_KEY_SIZE = 256;
inline constexpr size_t MAX_BUF_SIZE = std::numeric_limits<unsigned short>::max();
inline constexpr size_t BUF_NETWORK_RECV_SIZE = 1024;
inline constexpr size_t ADDR_BUF_SIZE = (sizeof(sockaddr_in) + 16) * 2;

inline constexpr NetworkObjectIdType OBJECT_ID_START = INVALID_SESSION_ID + 1;

inline constexpr std::endian NATIVE_ENDIAN = std::endian::native;
inline constexpr std::endian FBS_ENDIAN = std::endian::little; // flatbuffers 엔디안 형식

enum class IoType : uint32_t {
    SEND,
    RECV,
    CONNECT,
    DISCONNECT,
    ACCEPT,

    // UPDATE 세분화
        // NPC
    NPC_MOVE,
    NPC_DISPATCH_EVENT,
    REMOVE_NPC,
    UPDATE_NPC,
    REMOVE_TRIGGER,
    PROCESS_GAME_EVETN,
    SPAWN_ITEM,

    // GAME ROOM
    SCENE_TRANSITION_COUNTDOWN,
    REMOVE_PLAYER_IN_ROOM,
    CHECK_GAME_CONDITION,
    CHECK_SESSION_HEART_BEAT,
    GAMEROOM_CHECK_GAME_END,
    GAMEROOM_TRANSITION_STAGE,
};

enum class CollisionState : BYTE {
    NONE,
    ENTER,
    STAY,
    EXIT
};
