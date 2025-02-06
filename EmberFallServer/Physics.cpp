#include "pch.h"
#include "Physics.h"

Physics::Physics() { }

Physics::~Physics() { }

void Physics::SetTransform(const std::shared_ptr<Transform>& transform) { 
    mTransform = transform;
}

void Physics::Update(const float deltaTime) {
    if (mTransform.expired()) {
        return;
    }

    auto gravity = SimpleMath::Vector3{ 0.0f, -GRAVITY_FACTOR * deltaTime, 0.0f };
    mTransform.lock()->Translate(gravity);
}
