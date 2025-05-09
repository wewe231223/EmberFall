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
        IWeapon(uint16_t roomIdx, Packets::Weapon type, float damage=GameProtocol::Logic::DEFAULT_DAMAGE);
        virtual ~IWeapon();

    public:
        uint16_t GetRoomIdx() const;
        Packets::Weapon GetWeaponType() const;
        SimpleMath::Vector3 GetHitBoxSize() const;

        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) abstract;

    public:
        GameUnits::GameUnit<GameUnits::StandardTime> mAttackDelay{ };

    protected:
        bool mAttackable{ true };
        uint16_t mRoomIdx{ };
        float mDamage{ };
        SimpleMath::Vector3 mHitBox;

    private:
        Packets::Weapon mWeaponType;
    };

    class Fist : public IWeapon {
    public:
        Fist(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage = GameProtocol::Logic::DEFAULT_DAMAGE);
        virtual ~Fist();

    public:
        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) override;
    };

    class Spear : public IWeapon {
    public:
        Spear(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage = GameProtocol::Logic::DEFAULT_DAMAGE);
        virtual ~Spear();

    public:
        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) override;
    };

    class Bow : public IWeapon {
    public:
        Bow(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage = GameProtocol::Logic::DEFAULT_DAMAGE);
        virtual ~Bow();
    
    public:
        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) override;
    
    private:
        GameUnits::GameUnit<GameUnits::StandardSpeed> mArrowSpeed{ };
    };

    class Sword : public IWeapon {
    public:
        Sword(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage = GameProtocol::Logic::DEFAULT_DAMAGE);
        virtual ~Sword();
    
    public:
        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) override;
    };

    class Staff : public IWeapon {
    public:
        Staff(uint16_t roomIdx, const SimpleMath::Vector3& hitBoxSize, float damage = GameProtocol::Logic::DEFAULT_DAMAGE);
        virtual ~Staff();

    public:
        virtual void Attack(NetworkObjectIdType ownerId, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir) override;
    };
}