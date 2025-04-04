#include "pch.h"
#include "WeaponSystem.h"
#include "BoundingBoxImporter.h"

WeaponSystem::WeaponSystem(NetworkObjectIdType ownerId) 
    : mOwnerId{ ownerId } {
    SetWeapon(Weapon::NONE);
}

WeaponSystem::~WeaponSystem() { }

Weapon WeaponSystem::GetWeaponType() const {
    return mWeapon->GetWeaponType();
}

void WeaponSystem::Attack(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) { 
    mWeapon->Attack(mOwnerId, pos, dir);
}

void WeaponSystem::SetOwnerId(NetworkObjectIdType id) {
    mOwnerId = id;
}

void WeaponSystem::SetWeapon(Weapon weapon) {
    mWeapon.reset();

    switch (weapon) {
    case Weapon::NONE:
        mWeapon = std::make_shared<Weapons::Fist>(SimpleMath::Vector3{ 1.0f, 1.0f, 1.0f });
        break;

    case Weapon::SWORD:
        //mWeapon = std::make_shared<Weapons::Sword>(BoundingBoxImporter::GetBoundingBox(EntryKeys::SWORD_BOUNDING_BOX).Extents);
        mWeapon = std::make_shared<Weapons::Sword>(SimpleMath::Vector3{ 1.0f, 1.0f, 5.0f });
        break;

    case Weapon::SPEAR:
        mWeapon = std::make_shared<Weapons::Spear>(SimpleMath::Vector3{ 0.5f });
        break;

    case Weapon::BOW:
        mWeapon = std::make_shared<Weapons::Bow>(SimpleMath::Vector3{ 0.5f });
        break;

    case Weapon::STAFF:
        mWeapon = std::make_shared<Weapons::Staff>(SimpleMath::Vector3{ 0.5f });
        break;

    default:
        break;
    }
}
