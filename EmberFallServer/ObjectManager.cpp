#include "pch.h"
#include "ObjectManager.h"

#include "PlayerScript.h"
#include "MonsterScript.h"

#include "Input.h"
#include "Sector.h"


ObjectManager::ObjectManager() { }

ObjectManager::~ObjectManager() { }

void ObjectManager::Init() {
    for (NetworkObjectIdType id{ USER_ID_START }; auto & user : mPlayers) {
        user = std::make_shared<GameObject>();
        user->InitId(id);
        user->Reset();
        user->mSpec.active = false;

        ++id;
    }

    for (NetworkObjectIdType id{ MONSTER_ID_START }; auto& monster : mMonsters) {
        monster = std::make_shared<GameObject>();
        monster->InitId(id);
        monster->Reset();
        monster->mSpec.active = false;

        ++id;
    }

    for (NetworkObjectIdType id{ PROJECTILE_ID_START }; auto & projectile : mProjectiles) {
        projectile = std::make_shared<GameObject>();
        projectile->InitId(id);
        projectile->Reset();
        projectile->mSpec.active = false;

        ++id;
    }

    for (NetworkObjectIdType id{ ENV_ID_START }; auto & env : mEnvironments) {
        env = std::make_shared<GameObject>();
        env->InitId(id);
        env->Reset();
        env->mSpec.active = false;

        ++id;
    }
}

void ObjectManager::LoadEnvFromFile() { }

std::shared_ptr<GameObject> ObjectManager::GetObjectFromId(NetworkObjectIdType id) const {
    if (id > VALID_ID_MAX) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Object Array Access - Access Id: [{}]", id);
        return nullptr;
    }

    if (id < MONSTER_ID_START - 1) {
        return mPlayers[id];
    }
    else if (id < PROJECTILE_ID_START - 1) {
        return mMonsters[id - MONSTER_ID_START];
    }
    else if (id < ENV_ID_START - 1) {
        return mProjectiles[id - PROJECTILE_ID_START];
    }

    return mEnvironments[id - ENV_ID_START];
}

std::shared_ptr<GameObject> ObjectManager::GetPlayer(NetworkObjectIdType id) const {
    if (id >= MONSTER_ID_START or id < USER_ID_START) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Player Object Array Access - Access Id: [{}]", id);
        return nullptr;
    }

    return mPlayers[id];
}

std::shared_ptr<GameObject> ObjectManager::GetMonster(NetworkObjectIdType id) const {
    if (id >= ENV_ID_START or id < MONSTER_ID_START) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Monster Object Array Access - Access Id: [{}]", id);
        return nullptr;
    }

    return mMonsters[id - MONSTER_ID_START];
}

std::shared_ptr<GameObject> ObjectManager::GetEnv(NetworkObjectIdType id) const {
    if (id >= INVALID_OBJ_ID or id < ENV_ID_START) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad ENV Object Array Access - Access Id: [{}]", id);
        return nullptr;
    }

    return mEnvironments[id - ENV_ID_START];
}

bool ObjectManager::InViewRange(NetworkObjectIdType id1, NetworkObjectIdType id2, const float range) {
    const auto obj1 = GetObjectFromId(id1);
    const auto obj2 = GetObjectFromId(id2);

    const auto pos1 = obj1->GetPosition();
    const auto pos2 = obj2->GetPosition();

    auto dist = SimpleMath::Vector3::DistanceSquared(pos1, pos2);
    return dist < (range * range);
}

std::shared_ptr<GameObject> ObjectManager::SpawnObject(Packets::EntityType entity) {
    NetworkObjectIdType validId{ };
    switch (entity) {
    case Packets::EntityType_HUMAN:
    {
        break;
    }

    case Packets::EntityType_MONSTER:
    {
        if (false == mMonsterIndices.try_pop(validId)) {
            gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Max User");
            break;
        }

        auto obj = GetObjectFromId(validId);
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "validId is: {}, obj Id is: {}", validId, obj->GetId());
        obj->mSpec.active = true;
        obj->CreateScript<MonsterScript>(obj);
        obj->CreateBoundingObject<OBBCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f, 0.8f, 0.5f });
        obj->Init();
        
        gSectorSystem->AddInSector(validId, obj->GetPosition());
        obj->RegisterUpdate();

        return obj;
    }

    case Packets::EntityType_PROJECTILE:
    {
        break;
    }

    case Packets::EntityType_CORRUPTED_GEM:
    {
        break;
    }

    case Packets::EntityType_BOSS:
    {
        break;
    }

    default:
        break;
    }

    return nullptr;
}

void ObjectManager::ReleaseObject(NetworkObjectIdType id) {
    if (id > VALID_ID_MAX) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Object Array Access - Access Id: [{}]", id);
        return;
    }

    if (id < MONSTER_ID_START - 1) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Release Player Object");
        return;
    }
    else if (id < PROJECTILE_ID_START - 1) {
        return mMonsterIndices.push(id);
    }
    else if (id < ENV_ID_START - 1) {
        return mProjectileIndices.push(id);
    }

    return mEnvironmentsIndices.push(id);
}
