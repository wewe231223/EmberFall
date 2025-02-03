#include "pch.h"
#include "GameObject.h"

GameObject::GameObject() {
}

GameObject::~GameObject() {

}

void GameObject::SetInput(Key key) {
    mKeyInputs[mKeyInputSize++] = key;
}

void GameObject::Update(const float deltaTime) {
    SimpleMath::Vector3 moveVec{ SimpleMath::Vector3::Zero };
    for (size_t idx = 0; idx < mKeyInputSize; ++idx) {
        auto [key, state] = mKeyInputs[idx];
        if ('A' == key and true == state) {
            moveVec.x -= 1.0f;
        }

        if ('D' == key and true == state) {
            moveVec.x += 1.0f;
        }

        if ('W' == key and true == state) {
            moveVec.z += 1.0f;
        }

        if ('S' == key and true == state) {
            moveVec.z -= 1.0f;
        }

        if ('Q' == key and true == state) {
            moveVec.y -= 1.0f;
        }

        if ('E' == key and true == state) {
            moveVec.y += 1.0f;
        }
    }
    
    mTransform.Translate(moveVec);
    mKeyInputSize = 0;

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
