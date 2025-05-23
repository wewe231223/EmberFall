#pragma once

#include "../ServerLib/Session.h"
#include "GameObject.h"

inline constexpr uint8_t SESSION_CONNECT = 0b0000'0001;
inline constexpr uint8_t SESSION_INLOBBY = 0b0000'0010;
inline constexpr uint8_t SESSION_INGAME = 0b0000'0100;
inline constexpr uint8_t SESSION_CLOSE = 0b0000'1000;
inline constexpr uint8_t SESSION_STATE_NONE = 0b0001'0000;

inline constexpr uint8_t SESSION_READY_TO_RECV = SESSION_INLOBBY | SESSION_INGAME;

inline constexpr uint8_t PLAYER_ROLE_NONE = 0;
inline constexpr uint8_t PLAYER_ROLE_PLAYER = 1;
inline constexpr uint8_t PLAYER_ROLE_BOSS = 2;

class GameSession : public Session {
public:
    GameSession();
    ~GameSession();

public:
    bool GetReadyState() const;
    uint8_t GetSessionState() const;
    uint8_t GetSlotIndex() const;
    Packets::PlayerRole GetPlayerRole() const;
    std::string GetName() const;
    std::string_view GetNameView() const;
    std::shared_ptr<GameObject> GetUserObject() const;

    void SetSlotIndex(uint8_t slotIndex);
    void SetName(const std::string& str);

    void ChangeRole(Packets::PlayerRole role);
    bool ReadyToRecv() const;
    bool Ready();
    bool CancelReady();
    void EnterLobby();
    void EnterInGame();
    void InitUserObject();
    void InitPlayerScript();

    void UpdateViewList(const std::vector<NetworkObjectIdType>& inViewRangeNPC, const std::vector<NetworkObjectIdType>& inViewRangePlayer);

    virtual void Close() override;
    virtual void OnConnect() override;
    virtual void ProcessRecv(INT32 numOfBytes) override;

private:
    // for lobby
    SessionLobbyInfo mLobbyInfo{ false, 0, Packets::PlayerRole::PlayerRole_NONE };
    std::atomic_uint8_t mSessionState{ SESSION_STATE_NONE };

    std::string mName{ };
    std::shared_ptr<GameObject> mUserObject{ nullptr };

    std::shared_mutex mViewListLock{ };
    std::unordered_set<NetworkObjectIdType> mViewList{ };
};