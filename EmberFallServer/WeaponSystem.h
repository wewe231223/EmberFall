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
    WeaponSystem(NetworkObjectIdType ownerId, float damage=GameProtocol::Logic::DEFAULT_DAMAGE);
    ~WeaponSystem();

public:
    Packets::Weapon GetWeaponType() const;
    SimpleMath::Vector3 GetHitBoxSize() const;

    void SetOwnerId(uint16_t roomIdx, NetworkObjectIdType id);
    void SetWeapon(Packets::Weapon weapon, float damage);

    void Attack(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir);

private:
    uint16_t mRoomIdx{ };
    NetworkObjectIdType mOwnerId{ };
    std::shared_ptr<Weapons::IWeapon> mWeapon{ };
};