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

    if (Key::DOWN == mKeys[VK_F1]) {
        physics->mFactor.mess += 10.0f;
    }

    moveDir.Normalize();
    auto velocity = moveDir * physics->mFactor.maxMoveSpeed;

    physics->AddVelocity(velocity);
}

#ifdef _DEBUG
void Input::Update(float deltaTime, SessionIdType id) {
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

    if (Key::DOWN == mKeys[VK_F1]) {
        physics->mFactor.mess += 10.0f;
        std::cout << std::format("[ID: {}] DOWN F1 mess += 10.0f, currentMess: {}\n", id, physics->mFactor.mess);
    }

    moveDir.Normalize();
    auto velocity = moveDir * physics->mFactor.maxMoveSpeed;

    physics->AddVelocity(velocity);
}
#endif