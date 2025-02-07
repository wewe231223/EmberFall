#pragma once

#include "Physics.h"
#include "Transform.h"

class GameObjectComponent abstract : public std::enable_shared_from_this<GameObjectComponent> {
public:
    GameObjectComponent(const std::shared_ptr<Physics>& physics=nullptr, const std::shared_ptr<Transform>& transform=nullptr);
    virtual ~GameObjectComponent();

public:
    std::shared_ptr<Physics> GetPhysics() const;
    std::shared_ptr<Transform> GetTransform() const;

    virtual void Update(const float deltaTime) abstract;

private:
    std::weak_ptr<Physics> mPhysics{ };
    std::weak_ptr<Transform> mTransform{ };
};