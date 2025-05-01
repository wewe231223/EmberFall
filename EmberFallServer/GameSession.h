#pragma once

#include "../ServerLib/Session.h"
#include "GameObject.h"

inline constexpr uint8_t SESSION_CONNECT = 0;
inline constexpr uint8_t SESSION_INLOBBY = 1;
inline constexpr uint8_t SESSION_INGAME = 2;
inline constexpr uint8_t SESSION_CLOSE = 3;
inline constexpr uint8_t SESSION_STATE_NONE = 4;

class GameSession : public Session {
public:
    GameSession();
    ~GameSession();

public:
    std::shared_ptr<GameObject> GetUserObject() const;
    uint8_t GetSessionState() const;

    void InitUserObject();
    void EnterLobby();
    void EnterInGame();

    virtual void OnConnect() override;
    virtual void ProcessRecv(INT32 numOfBytes) override;

private:
    std::atomic_uint8_t mSessionState{ SESSION_STATE_NONE };
    std::shared_ptr<GameObject> mUserObject{ nullptr };
};