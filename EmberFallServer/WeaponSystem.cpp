#include "pch.h"
#include "WeaponSystem.h"
#include "BoundingBoxImporter.h"

WeaponSystem::WeaponSystem(NetworkObjectIdType ownerId) 
    : mOwnerId{ ownerId } {
    SetWeapon(Packets::Weapon_SWORD);
}

WeaponSystem::~WeaponSystem() { }

Packets::Weapon WeaponSystem::GetWeaponType() const {
    return mWeapon->GetWeaponType();
}

void WeaponSystem::Attack(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) { 
    mWeapon->Attack(mOwnerId, pos, dir);
}

void WeaponSystem::SetOwnerId(NetworkObjectIdType id) {
    mOwnerId = id;
}

void WeaponSystem::SetWeapon(Packets::Weapon weapon) {
    mWeapon.reset();

    switch (weapon) {
    case Packets::Weapon_SWORD:
        mWeapon = std::make_shared<Weapons::Sword>(SimpleMath::Vector3{ 1.0f, 1.0f, 5.0f });
        break;

    case Packets::Weapon_SPEAR:
        mWeapon = std::make_shared<Weapons::Spear>(SimpleMath::Vector3{ 0.5f });
        break;

    case Packets::Weapon_BOW:
        mWeapon = std::make_shared<Weapons::Bow>(SimpleMath::Vector3{ 0.5f });
        break;

    case Packets::Weapon_STAFF:
        mWeapon = std::make_shared<Weapons::Staff>(SimpleMath::Vector3{ 0.5f });
        break;

    default:
        break;
    }
}
