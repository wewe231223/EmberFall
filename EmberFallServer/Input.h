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

    void Update(float deltaTime);

private:
    std::array<bool, MAX_KEY_SIZE> mKeys{ };
};