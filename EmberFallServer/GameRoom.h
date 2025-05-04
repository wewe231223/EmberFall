#pragma once

#include "GameSession.h"
#include "Stage.h"

inline constexpr uint8_t GAME_ROOM_STATE_LOBBY = 0;
inline constexpr uint8_t GAME_ROOM_STATE_INGAME = 1;

inline constexpr uint16_t INSERT_GAME_ROOM_ERROR = 0xFFFF;

namespace GameRoomError {
    inline constexpr uint8_t SUCCESS_INSERT_SESSION_IN_ROOM = 0;
    inline constexpr uint8_t SUCCESS_REMOVE_SESSION_IN_ROOM = 1;
    inline constexpr uint8_t ERROR_MAX_SESSION_IN_ONE_ROOM = 2;
    inline constexpr uint8_t ERROR_ROOM_STATE_IS_INGAME = 3;
    inline constexpr uint8_t ERROR_SESSION_EXISTS_IN_THIS_ROOM = 4;
    inline constexpr uint8_t ERROR_SESSION_NOT_EXISTS_IN_THIS_ROOM = 5;
    inline constexpr uint8_t ERROR_ALL_ROOM_IS_FULL = 6;
}


class GameRoom {
private:
    static constexpr size_t MAX_PLAYER_IN_GAME_ROOM = 5;
    static constexpr float SCENE_TRANSITION_COUNT = 5.0f; // second
    static constexpr std::chrono::milliseconds SCENE_TRANSITION_EVENT_DELAY = 500ms;

public:
    using SessionListInRoom = std::unordered_set<SessionIdType>;

public:
    GameRoom(uint16_t mRoomIdx);
    ~GameRoom();

public:
    bool IsEveryPlayerReady() const;
    bool IsMaxSession() const;
    uint8_t GetGameRoomState() const;
    Stage& GetStage();
    Lock::SRWLock& GetSessionLock();
    SessionListInRoom& GetSessions();

    uint8_t TryInsertInRoom(SessionIdType session);
    uint8_t RemovePlayer(SessionIdType id, Packets::PlayerRole lastRole, bool lastReadyState, uint8_t lastSlotIndex);

    bool ChangeRolePlayer(SessionIdType id, Packets::PlayerRole role);
    bool ReadyPlayer(SessionIdType id);
    bool CancelPlayerReady(SessionIdType id);
    bool CheckAndStartGame();

    void BroadCastInGameRoom(SessionIdType sender, OverlappedSend* packet);
    void BroadCastInGameRoom(OverlappedSend* packet);

    void OnSceneCountdownTick();

private:
    uint16_t mRoomIdx{ };

    mutable Lock::SRWLock mSessionLock{ };

    std::atomic_uint8_t mGameRoomState{ };
    std::atomic_uint8_t mBossPlayerCount{ };
    uint8_t mPlayerCount{ };
    uint8_t mReadyPlayerCount{ };

    SysClock::time_point mSceneTransitionCounter{ };

    SessionListInRoom mSessionsInRoom{ };   
    Concurrency::concurrent_queue<uint8_t> mSessionSlotIndices{ };

    Stage mStage;
};

using SessionListInRoom = GameRoom::SessionListInRoom;

class GameRoomManager {
private:
    static constexpr size_t MAX_GAME_ROOM = 50;
    inline static uint8_t LAST_ERROR_CODE = 0;

public:
    GameRoomManager();
    ~GameRoomManager();

public:
    static uint8_t GetLastErrorCode();
    std::unique_ptr<GameRoom>& GetRoom(uint16_t roomIdx);

    void InitGameRooms();

    uint16_t TryInsertGameRoom(SessionIdType session);
    uint8_t TryRemoveGameRoom(uint16_t roomIdx, SessionIdType sessionId, Packets::PlayerRole lastRole, bool lastReadyState, uint8_t lastSlotIndex);

    Lock::SRWLock& GetSessionLock(uint16_t roomIdx);
    SessionListInRoom& GetSessionsInRoom(uint16_t roomIdx);

    bool ChangeRolePlayer(uint16_t roomIdx, SessionIdType id, Packets::PlayerRole role);
    bool ReadyPlayer(uint16_t roomIdx, SessionIdType id);
    bool CancelPlayerReady(uint16_t roomIdx, SessionIdType id);

private:
    Lock::SRWLock mGameRoomLock{ };
    std::array<std::unique_ptr<GameRoom>, MAX_GAME_ROOM> mGameRooms;
};