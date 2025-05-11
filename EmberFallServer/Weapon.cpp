#include "pch.h"
#include "Weapon.h"
#include "ObjectManager.h"
#include "GameRoom.h"

using namespace Weapons;

IWeapon::IWeapon(uint16_t roomIdx, float damage)
    : mRoomIdx{ roomIdx }, mDamage{ damage } { }

IWeapon::~IWeapon() { }

uint16_t Weapons::IWeapon::GetRoomIdx() const {
    return mRoomIdx;
}

SimpleMath::Vector3 Weapons::IWeapon::GetHitBoxSize() const {
    return mHitBox;
}

Fist::Fist(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage)
    : IWeapon{ roomIdx, damage } {
    mHitBox = hitBoxSize;
}

Fist::~Fist() { }

void Fist::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = mDamage;
    event->knockBackForce = dir * 5000.0f;

    auto attackPos = pos + dir * mHitBox.Length();
    gGameRoomManager->GetRoom(GetRoomIdx())->GetStage().GetObjectManager()->SpawnEventTrigger(attackPos, mHitBox, dir, 1.5f, event, 1.5f, 1);
}

Spear::Spear(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage)
    : IWeapon{ roomIdx, damage } {
    mHitBox = hitBoxSize;
}

Spear::~Spear() { }

void Spear::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = mDamage;

    auto attackPos = pos + dir * mHitBox.Length();
    gGameRoomManager->GetRoom(GetRoomIdx())->GetStage().GetObjectManager()->SpawnEventTrigger(attackPos, mHitBox, dir, 1.5f, event, 1.5f, 1);
}

Bow::Bow(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage)
    : IWeapon{ roomIdx, damage }, mArrowSpeed{ GameProtocol::Unit::DEFAULT_PROJECTILE_SPEED } {
}

Bow::~Bow() { }

void Bow::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
}

Sword::Sword(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage)
    : IWeapon{ roomIdx, damage } {
    mHitBox = hitBoxSize;
}

Sword::~Sword() { }

void Sword::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = mDamage;
    event->knockBackForce = dir * 5000.0f;

    auto attackPos = pos + dir * mHitBox.z;
    gGameRoomManager->GetRoom(GetRoomIdx())->GetStage().GetObjectManager()->SpawnEventTrigger(attackPos, mHitBox, dir, 1.5f, event, 1.5f, 1);
}

Staff::Staff(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage)
    : IWeapon{ roomIdx, damage } {
    mHitBox = hitBoxSize;
}

Staff::~Staff() { }

void Staff::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = mDamage;

    //auto attackPos = pos + dir * mHitbox->GetForwardExtents();
    //gObjectSpawner->SpawnEventTrigger(event, 0.5f, 0.5f, 5, attackPos, dir, mHitbox);
}

Weapons::BossPlayerSword::BossPlayerSword(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage)
    : IWeapon{ roomIdx, damage } {
    mHitBox = hitBoxSize;
}

Weapons::BossPlayerSword::~BossPlayerSword() { }

void Weapons::BossPlayerSword::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = mDamage;
    event->knockBackForce = dir * 15000.0f;

    auto attackPos = pos + dir * mHitBox.z;
    gGameRoomManager->GetRoom(GetRoomIdx())->GetStage().GetObjectManager()->SpawnEventTrigger(attackPos, mHitBox, dir, 3.0f, event, 3.0f, 1);
}
