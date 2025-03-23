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
    WeaponSystem();
    ~WeaponSystem();

public:
    Weapon GetWeaponType() const;

    void Attack(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir);
    void SetWeapon(Weapon weapon);

private:
    std::shared_ptr<Weapons::IWeapon> mWeapon{ };
};