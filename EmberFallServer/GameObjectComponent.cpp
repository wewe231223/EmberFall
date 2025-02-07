#include "pch.h"
#include "GameObjectComponent.h"

GameObjectComponent::GameObjectComponent(const std::shared_ptr<Physics>& physics, const std::shared_ptr<Transform>& transform) 
    : mPhysics{ physics }, mTransform{ transform } { }

GameObjectComponent::~GameObjectComponent() { }

std::shared_ptr<Physics> GameObjectComponent::GetPhysics() const {
    if (mPhysics.expired()) {
        return nullptr;
    }
    return mPhysics.lock();
}

std::shared_ptr<Transform> GameObjectComponent::GetTransform() const {
    if (mTransform.expired()) {
        return nullptr;
    }
    return mTransform.lock();
}
