#include "pch.h"
#include "Weapon.h"

IWeapon::IWeapon(Weapon tag) 
    : mWeaponTag{ tag } { }

IWeapon::~IWeapon() { }
