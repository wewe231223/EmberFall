#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Input.h
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameObjectComponent.h"

enum class KeyState : BYTE {
    DOWN,
    PRESS,
    UP,
    RELEASE,
};

class Input : public GameObjectComponent {
public:
    Input(const std::shared_ptr<Physics>& physics, const std::shared_ptr<Transform>& transform);
    ~Input();

public:
    void UpdateInput(Key key);
    void UpdateInput(BYTE key, bool state);
    bool GetState(BYTE key) const;

    virtual void Update(float deltaTime) override;
#ifdef _DEBUG
    // 디버그용 함수
    void Update(float deltaTime, SessionIdType id);
#endif

private:
    std::array<bool, MAX_KEY_SIZE> mKeys{ };
};