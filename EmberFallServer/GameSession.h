#pragma once

#include "../ServerLib/Session.h"
#include "GameObject.h"

class GameSession : public Session {
public:
    GameSession();
    ~GameSession();

public:
    std::shared_ptr<GameObject> GetUserObject() const;

    virtual void OnConnect() override;
    virtual void ProcessRecv(INT32 numOfBytes) override;

private:
    void InitUserObject();

private:
    std::shared_ptr<GameObject> mUserObject{ nullptr };
};