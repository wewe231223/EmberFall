#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WeaponSystem.h
// 
// 2025 - 03 - 19 : 무기 관련 클래스
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Weapon.h"

class WeaponSystem {
public:
    WeaponSystem(NetworkObjectIdType ownerId);
    ~WeaponSystem();

public:
    Packets::Weapon GetWeaponType() const;

    void SetOwnerId(NetworkObjectIdType id);
    void SetWeapon(Packets::Weapon weapon);

    void Attack(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir);

private:
    NetworkObjectIdType mOwnerId{ };
    std::shared_ptr<Weapons::IWeapon> mWeapon{ };
};