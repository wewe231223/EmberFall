#include "pch.h"
#include "Collider.h"

Collider::Collider(ColliderType type) 
    : mType{ type } { }

Collider::~Collider() { }

Collider::Collider(const Collider& other) 
    : mState{ other.mState } { }

Collider::Collider(Collider&& other) noexcept 
    : mState{ other.mState } { }

Collider& Collider::operator=(const Collider& other) {
    mState = other.mState;
    return *this;
}

Collider& Collider::operator=(Collider&& other) noexcept {
    mState = other.mState;
    return *this;
}
