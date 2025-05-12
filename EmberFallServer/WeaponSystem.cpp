#include "pch.h"
#include "WeaponSystem.h"
#include "BoundingBoxImporter.h"
#include "Resources.h"

WeaponSystem::WeaponSystem(NetworkObjectIdType ownerId, float damage)
    : mOwnerId{ ownerId } {
    SetWeapon(Packets::EntityType_HUMAN_LONGSWORD, damage);
}

WeaponSystem::~WeaponSystem() { }

SimpleMath::Vector3 WeaponSystem::GetHitBoxSize() const {
    return mWeapon->GetHitBoxSize();
}

void WeaponSystem::Attack(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) { 
    mWeapon->Attack(mOwnerId, pos, dir);
}

void WeaponSystem::SetOwnerId(uint16_t roomIdx, NetworkObjectIdType id) {
    mRoomIdx = roomIdx;
    mOwnerId = id;
}

void WeaponSystem::SetWeapon(Packets::EntityType type, float damage) {
    mWeapon.reset();

    switch (type) {
    case Packets::EntityType_HUMAN_LONGSWORD:
        mWeapon = std::make_shared<Weapons::LongSword>(mRoomIdx, SimpleMath::Vector3{ 0.7f, 0.7f, 2.0f }, damage);
        break;

    case Packets::EntityType_HUMAN_SWORD:
        mWeapon = std::make_shared<Weapons::Sword>(mRoomIdx, SimpleMath::Vector3{ 0.7f, 0.7f, 1.5f }, damage);
        break;

    case Packets::EntityType_HUMAN_ARCHER:
        mWeapon = std::make_shared<Weapons::Bow>(mRoomIdx, SimpleMath::Vector3{ 0.5f }, damage);
        break;

    case Packets::EntityType_HUMAN_MAGICIAN:
        mWeapon = std::make_shared<Weapons::Staff>(mRoomIdx, SimpleMath::Vector3{ 0.5f }, damage);
        break;

    case Packets::EntityType_BOSS:
        mWeapon = std::make_shared<Weapons::BossPlayerSword>(mRoomIdx, SimpleMath::Vector3{ 1.3f, 1.3f, 5.0f }, damage);
        break;

    case Packets::EntityType_MONSTER:
        mWeapon = std::make_shared<Weapons::Fist>(mRoomIdx, SimpleMath::Vector3{ 0.7f, 0.7f, 2.0f });
        break;

    default:
        break;
    }
}
