#include "pch.h"
#include "Weapon.h"
#include "Collider.h"

using namespace Weapons;

IWeapon::IWeapon(Weapon type) 
    : mWeaponType{ type } { }

IWeapon::~IWeapon() { }

Weapon Weapons::IWeapon::GetWeaponType() const {
    return mWeaponType;
}
//
//std::shared_ptr<class Collider> IWeapon::GetHitbox() const {
//    return std::shared_ptr<class Collider>();
//}

Spear::Spear(const SimpleMath::Vector3& hitBoxSize)
    : IWeapon{ Weapon::SPEAR } {
    //mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Spear::~Spear() { }

void Spear::Attack(const SimpleMath::Vector3& dir) { }

Bow::Bow(const SimpleMath::Vector3& hitBoxSize, GameUnits::GameUnit<GameUnits::StandardSpeed> arrowSpeed)
    : IWeapon{ Weapon::BOW }, mArrowSpeed{ arrowSpeed } {
    //mHitbox = std::make_shared<OrientedBoxCollider>(SimpleMath::Vector3::Zero, hitBoxSize);
}

Bow::~Bow() { }

void Bow::Attack(const SimpleMath::Vector3& dir)
{
}
