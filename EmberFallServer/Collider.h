#pragma once

enum class CollisionState : BYTE {
    NONE,
    ENTER,
    STAY,
    EXIT
};

class Collider {
public:
    Collider();
    ~Collider();

    Collider(const Collider& other);
    Collider(Collider&& other) noexcept;
    Collider& operator=(const Collider& other);
    Collider& operator=(Collider&& other) noexcept;

public:
    virtual bool CheckCollision(Collider& other) abstract;

    virtual void OnCollisionEnter() abstract;
    virtual void OnCollisionStay() abstract;
    virtual void OnCollisionExit() abstract;

protected:
    CollisionState mState{ CollisionState::NONE };
};