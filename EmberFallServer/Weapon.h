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
        IWeapon(Weapon type);
        virtual ~IWeapon();

    public:
        Weapon GetWeaponType() const;
        std::shared_ptr<Collider> GetHitbox() const;

        virtual void Attack(const SimpleMath::Vector3& dir) abstract;

    public:
        GameUnits::GameUnit<GameUnits::StandardTime> mAttackDelay{ };

    protected:
        bool mAttackable{ true };
        float mDamage{ };
        std::shared_ptr<Collider> mHitbox{ };

    private:
        Weapon mWeaponType;
    };

    class Spear : public IWeapon {
    public:
        Spear(const SimpleMath::Vector3& hitBoxSize);
        virtual ~Spear();

    public:
        virtual void Attack(const SimpleMath::Vector3& dir) override;
    };

    class Bow : public IWeapon {
        Bow(const SimpleMath::Vector3& hitBoxSize, GameUnits::GameUnit<GameUnits::StandardSpeed> arrowSpeed);
        virtual ~Bow();
    
    public:
        virtual void Attack(const SimpleMath::Vector3& dir) override;
    
    private:
        GameUnits::GameUnit<GameUnits::StandardSpeed> mArrowSpeed{ };
    };

    class Sword : public IWeapon {
        Sword(const SimpleMath::Vector3& hitBoxSize);
        virtual ~Sword();
    
    public:
        virtual void Attack(const SimpleMath::Vector3& dir) override;
    };
}