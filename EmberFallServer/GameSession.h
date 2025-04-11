#pragma once

#include "../ServerLib/Session.h"
#include "GameObject.h"

class GameSession : public Session {
public:
    GameSession(std::shared_ptr<INetworkCore> core);
    ~GameSession();

public:
    virtual void ProcessRecv(INT32 numOfBytes) override;

    std::shared_ptr<GameObject> GetUserObject() const;

private:
    std::shared_ptr<GameObject> mUserObject{ nullptr };
};