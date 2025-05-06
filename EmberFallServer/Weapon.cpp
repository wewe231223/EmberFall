#include "pch.h"
#include "Weapon.h"
#include "ObjectManager.h"
#include "GameRoom.h"

using namespace Weapons;

IWeapon::IWeapon(uint16_t roomIdx, Packets::Weapon type)
    : mWeaponType{ type }, mRoomIdx{ roomIdx } { }

IWeapon::~IWeapon() { }

uint16_t Weapons::IWeapon::GetRoomIdx() const {
    return mRoomIdx;
}

Packets::Weapon IWeapon::GetWeaponType() const {
    return mWeaponType;
}

SimpleMath::Vector3 Weapons::IWeapon::GetHitBoxSize() const {
    return mHitBox;
}

Fist::Fist(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ roomIdx, Packets::Weapon_SWORD } {
    mHitBox = hitBoxSize;
}

Fist::~Fist() { }

void Fist::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;
    event->knockBackForce = dir * 5000.0f;

    /*auto attackPos = pos + dir * mHitbox->GetForwardExtents();
    gGameRoomManager->GetRoom(GetRoomIdx())->GetObjectManager()->SpawnEventTrigger(event, 0.5f, 0.5f, 1, attackPos, dir, mHitbox);*/
}

Spear::Spear(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ roomIdx, Packets::Weapon_SPEAR } {
    mHitBox = hitBoxSize;
}

Spear::~Spear() { }

void Spear::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;

    auto attackPos = pos + dir * mHitBox.Length();
    gGameRoomManager->GetRoom(GetRoomIdx())->GetStage().GetObjectManager()->SpawnEventTrigger(attackPos, mHitBox, dir, 0.5f, event, 0.5f, 1);
}

Bow::Bow(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ roomIdx, Packets::Weapon_BOW }, mArrowSpeed{ GameProtocol::Unit::DEFAULT_PROJECTILE_SPEED } {
}

Bow::~Bow() { }

void Bow::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
}

Sword::Sword(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ roomIdx, Packets::Weapon_SWORD } {
    mHitBox = hitBoxSize;
}

Sword::~Sword() { }

void Sword::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;
    event->knockBackForce = dir * 5000.0f;

    auto attackPos = pos + dir * mHitBox.z * 0.5f;
    gGameRoomManager->GetRoom(GetRoomIdx())->GetStage().GetObjectManager()->SpawnEventTrigger(attackPos, mHitBox, dir, 0.5f, event, 0.5f, 1);
}

Staff::Staff(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ roomIdx, Packets::Weapon_STAFF } {
    mHitBox = hitBoxSize;
}

Staff::~Staff() { }

void Staff::Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) {
    std::shared_ptr<AttackEvent> event = std::make_shared<AttackEvent>();
    event->sender = ownerId;
    event->damage = GameProtocol::Logic::DEFAULT_DAMAGE;

    //auto attackPos = pos + dir * mHitbox->GetForwardExtents();
    //gObjectSpawner->SpawnEventTrigger(event, 0.5f, 0.5f, 5, attackPos, dir, mHitbox);
}
