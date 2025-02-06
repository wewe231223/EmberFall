#pragma once

#include "Transform.h"

// Physics Base
class Physics { 
public:
    inline static constexpr float GRAVITY_FACTOR = 100.0f;

public:
    Physics();
    ~Physics();

public:
    void SetTransform(const std::shared_ptr<Transform>& transform);

    void Update(const float deltaTime);

private:
    std::weak_ptr<Transform> mTransform{ };
};
