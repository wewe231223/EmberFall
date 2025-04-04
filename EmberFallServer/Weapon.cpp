#include "pch.h"
#include "Weapon.h"
#include "ObjectSpawner.h"

using namespace Weapons;

IWeapon::IWeapon(Weapon type) 
    : mWeaponType{ type } { }

IWeapon::~IWeapon() { }

Weapon IWeapon::GetWeaponType() const {
    return mWeaponType;
}
std::shared_ptr<Collider> IWeapon::GetHitbox() const {
    return mHitbox;
}

Fist::Fist(const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ Weapon::NONE } { 
    mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Fist::~Fist() { }

void Fist::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;
    event->knockBackForce = dir * 5000.0f;

    auto attackPos = pos + SimpleMath::Vector3::Transform(SimpleMath::Vector3::Forward * mHitbox->GetForwardExtents(),
         SimpleMath::Quaternion::FromToRotation(SimpleMath::Vector3::Forward, dir));
    gObjectSpawner->SpawnEventTrigger(event, 2.3f, 2.3f, 1, attackPos, dir, mHitbox);
}

Spear::Spear(const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ Weapon::SPEAR } {
    mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Spear::~Spear() { }

void Spear::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;

    auto attackPos = pos + SimpleMath::Vector3::Transform(SimpleMath::Vector3::Forward * mHitbox->GetForwardExtents(),
        SimpleMath::Quaternion::FromToRotation(SimpleMath::Vector3::Forward, dir));
    gObjectSpawner->SpawnEventTrigger(event, 5.0f, 1.0f, 5, attackPos, dir, mHitbox);
}

Bow::Bow(const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ Weapon::BOW }, mArrowSpeed{ GameProtocol::Unit::DEFAULT_PROJECTILE_SPEED } {
    mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Bow::~Bow() { }

void Bow::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    gObjectSpawner->SpawnProjectile(ObjectTag::ARROW, pos, dir, mArrowSpeed);
}

Sword::Sword(const SimpleMath::Vector3& hitBoxSize) 
    : IWeapon{ Weapon::SWORD } {
    mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Sword::~Sword() { }

void Sword::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;
    event->knockBackForce = dir * 5000.0f;

    auto attackPos = pos + SimpleMath::Vector3::Transform(SimpleMath::Vector3::Forward * mHitbox->GetForwardExtents(),
        SimpleMath::Quaternion::FromToRotation(SimpleMath::Vector3::Forward, dir));
    gObjectSpawner->SpawnEventTrigger(event, 2.3f, 2.3f, 1, attackPos, dir, mHitbox);
}

Staff::Staff(const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ Weapon::STAFF } {
    mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Staff::~Staff() { }

void Staff::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;

    auto attackPos = pos + SimpleMath::Vector3::Transform(SimpleMath::Vector3::Forward * mHitbox->GetForwardExtents(),
        SimpleMath::Quaternion::FromToRotation(SimpleMath::Vector3::Forward, dir));
    gObjectSpawner->SpawnEventTrigger(event, 5.0f, 1.0f, 5, attackPos, dir, mHitbox);
}
