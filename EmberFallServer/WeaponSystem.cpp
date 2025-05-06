#include "pch.h"
#include "WeaponSystem.h"
#include "BoundingBoxImporter.h"
#include "Resources.h"

WeaponSystem::WeaponSystem(NetworkObjectIdType ownerId) 
    : mOwnerId{ ownerId } {
    SetWeapon(Packets::Weapon_SWORD);
}

WeaponSystem::~WeaponSystem() { }

Packets::Weapon WeaponSystem::GetWeaponType() const {
    return mWeapon->GetWeaponType();
}

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

void WeaponSystem::SetWeapon(Packets::Weapon weapon) {
    mWeapon.reset();

    switch (weapon) {
    case Packets::Weapon_SWORD:
        mWeapon = std::make_shared<Weapons::Sword>(mRoomIdx, SimpleMath::Vector3{ 0.5f, 0.5f, 1.5f });
        break;

    case Packets::Weapon_SPEAR:
        mWeapon = std::make_shared<Weapons::Spear>(mRoomIdx, SimpleMath::Vector3{ 0.5f });
        break;

    case Packets::Weapon_BOW:
        mWeapon = std::make_shared<Weapons::Bow>(mRoomIdx, SimpleMath::Vector3{ 0.5f });
        break;

    case Packets::Weapon_STAFF:
        mWeapon = std::make_shared<Weapons::Staff>(mRoomIdx, SimpleMath::Vector3{ 0.5f });
        break;

    default:
        break;
    }
}
