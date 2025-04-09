#include "pch.h"
#include "Weapon.h"
#include "ObjectSpawner.h"

using namespace Weapons;

IWeapon::IWeapon(Packets::Weapon type)
    : mWeaponType{ type } { }

IWeapon::~IWeapon() { }

Packets::Weapon IWeapon::GetWeaponType() const {
    return mWeaponType;
}
std::shared_ptr<Collider> IWeapon::GetHitbox() const {
    return mHitbox;
}

Fist::Fist(const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ Packets::Weapon_SWORD } {
    mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Fist::~Fist() { }

void Fist::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;
    event->knockBackForce = dir * 5000.0f;

    auto attackPos = pos + dir * mHitbox->GetForwardExtents();
    gObjectSpawner->SpawnEventTrigger(event, 0.5f, 0.5f, 1, attackPos, dir, mHitbox);
}

Spear::Spear(const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ Packets::Weapon_SPEAR } {
    mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Spear::~Spear() { }

void Spear::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;

    auto attackPos = pos + dir * mHitbox->GetForwardExtents();
    gObjectSpawner->SpawnEventTrigger(event, 0.5f, 0.5f, 1, attackPos, dir, mHitbox);
}

Bow::Bow(const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ Packets::Weapon_BOW }, mArrowSpeed{ GameProtocol::Unit::DEFAULT_PROJECTILE_SPEED } {
    mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Bow::~Bow() { }

void Bow::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    gObjectSpawner->SpawnProjectile(ObjectTag::ARROW, pos, dir, mArrowSpeed);
}

Sword::Sword(const SimpleMath::Vector3& hitBoxSize) 
    : IWeapon{ Packets::Weapon_SWORD } {
    mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Sword::~Sword() { }

void Sword::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;
    event->knockBackForce = dir * 5000.0f;

    auto attackPos = pos + dir * mHitbox->GetForwardExtents();
    gObjectSpawner->SpawnEventTrigger(event, 0.5f, 0.5f, 1, attackPos, dir, mHitbox);
}

Staff::Staff(const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ Packets::Weapon_STAFF } {
    mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Staff::~Staff() { }

void Staff::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;

    auto attackPos = pos + dir * mHitbox->GetForwardExtents();
    gObjectSpawner->SpawnEventTrigger(event, 0.5f, 0.5f, 5, attackPos, dir, mHitbox);
}
