#include "pch.h"
#include "Input.h"

Input::Input(const std::shared_ptr<Physics>& physics, const std::shared_ptr<Transform>& transform) 
    : GameObjectComponent{ physics, transform } {
    mKeys.fill(Key::UP);
}

Input::~Input() { }

void Input::UpdateInput(Key key) {
    UpdateInput(key.key, key.state);
}

void Input::UpdateInput(BYTE key, bool state) {
    mKeys[key] = state;
}

bool Input::GetState(BYTE key) const {
    return mKeys[key];
}

void Input::Update(float deltaTime) {
    auto physics = GetPhysics();
    if (nullptr == physics) {
        return;
    }
    
    SimpleMath::Vector3 moveDir{ SimpleMath::Vector3::Zero };
    if (Key::DOWN == mKeys['A']) {
        moveDir.x -= 1.0f;
    }

    if (Key::DOWN == mKeys['D']) {
        moveDir.x += 1.0f;
    }

    if (Key::DOWN == mKeys['W']) {
        moveDir.z -= 1.0f;
    }

    if (Key::DOWN == mKeys['S']) {
        moveDir.z += 1.0f;
    }

    if (Key::DOWN == mKeys[VK_SPACE]) {
        physics->Jump(deltaTime);
    }

    moveDir.Normalize();
    auto velocity = moveDir * physics->GetMaxMoveSpeed();
    
    physics->SetMoveVelocity(velocity);
}
