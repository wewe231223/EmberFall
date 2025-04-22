#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Weapon.h
// 
// 2025 - 03 - 14 : 무기 관련 클래스
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Collider.h"

namespace Weapons {
    class IWeapon abstract {
    public:
        IWeapon(Packets::Weapon type);
        virtual ~IWeapon();

    public:
        Packets::Weapon GetWeaponType() const;

        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) abstract;

    public:
        GameUnits::GameUnit<GameUnits::StandardTime> mAttackDelay{ };

    protected:
        bool mAttackable{ true };
        float mDamage{ };
        SimpleMath::Vector3 mHitBox;

    private:
        Packets::Weapon mWeaponType;
    };

    class Fist : public IWeapon {
    public:
        Fist(const SimpleMath::Vector3& hitBoxSize);
        virtual ~Fist();

    public:
        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) override;
    };

    class Spear : public IWeapon {
    public:
        Spear(const SimpleMath::Vector3& hitBoxSize);
        virtual ~Spear();

    public:
        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) override;
    };

    class Bow : public IWeapon {
    public:
        Bow(const SimpleMath::Vector3& hitBoxSize);
        virtual ~Bow();
    
    public:
        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) override;
    
    private:
        GameUnits::GameUnit<GameUnits::StandardSpeed> mArrowSpeed{ };
    };

    class Sword : public IWeapon {
    public:
        Sword(const SimpleMath::Vector3& hitBoxSize);
        virtual ~Sword();
    
    public:
        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) override;
    };

    class Staff : public IWeapon {
    public:
        Staff(const SimpleMath::Vector3& hitBoxSize);
        virtual ~Staff();

    public:
        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) override;
    };
}