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
    for (size_t idx = 0; idx < mKeyInputSize; ++idx) {
        auto [key, state] = mKeyInputs[idx];
        if ('A' == key and true == state) {
            mPosition.x -= 1.0f;
        }

        if ('D' == key and true == state) {
            mPosition.x -= 1.0f;
        }

        if ('W' == key and true == state) {
            mPosition.z -= 1.0f;
        }

        if ('S' == key and true == state) {
            mPosition.z -= 1.0f;
        }

        if ('Q' == key and true == state) {
            mPosition.y -= 1.0f;
        }

        if ('E' == key and true == state) {
            mPosition.y -= 1.0f;
        }
    }
}

SimpleMath::Matrix GameObject::GetMatrix() const {
    return mWorldMat;
}
