#include "pch.h"
#include "GameObject.h"

GameObject::GameObject() {
}

GameObject::~GameObject() {

}

void GameObject::SetInput(Key key) {
    mKeyState[key.key] = key.state;
}

void GameObject::Update(const float deltaTime) {
    SimpleMath::Vector3 moveVec{ SimpleMath::Vector3::Zero };
    if (Key::DOWN == mKeyState['A']) {
        moveVec.x -= TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState['D']) {
        moveVec.x += TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState['W']) {
        moveVec.z -= TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState['S']) {
        moveVec.z += TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState['Q']) {
        moveVec.y -= TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState['E']) {
        moveVec.y += TEMP_SPEED * deltaTime;
    }
    
    mTransform.Move(moveVec);

    mTransform.Update();
}

void GameObject::InitId(SessionIdType id) {
    mId = id;
}

SessionIdType GameObject::GetId() const {
    return mId;
}

SimpleMath::Matrix GameObject::GetWorld() const {
    return mTransform.GetWorld();
}

Transform& GameObject::GetTransform() {
    return mTransform;
}

SimpleMath::Vector3 GameObject::GetPosition() const {
    return mTransform.GetPosition();
}

SimpleMath::Quaternion GameObject::GetRotation() const {
    return mTransform.GetRotation();
}

SimpleMath::Vector3 GameObject::GetScale() const {
    return mTransform.GetScale();
}
