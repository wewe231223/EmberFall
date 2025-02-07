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
    
    SimpleMath::Vector3 moveVec{ SimpleMath::Vector3::Zero };
    if (Key::DOWN == mKeys['A']) {
        moveVec.x -= 1.0f;
    }

    if (Key::DOWN == mKeys['D']) {
        moveVec.x += 1.0f;
    }

    if (Key::DOWN == mKeys['W']) {
        moveVec.z -= 1.0f;
    }

    if (Key::DOWN == mKeys['S']) {
        moveVec.z += 1.0f;
    }

    if (Key::DOWN == mKeys[VK_SPACE]) {
        physics->Jump();
    }

    moveVec.Normalize();
    moveVec *= physics->GetMoveSpeed();
    
    physics->SetSpeed(moveVec);
}
