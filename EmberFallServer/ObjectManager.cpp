#include "pch.h"
#include "ObjectManager.h"

#include "PlayerScript.h"
#include "MonsterScript.h"

#include "Input.h"


ObjectManager::ObjectManager() {
    for (NetworkObjectIdType id = 0; id < MAX_USER + MAX_MONSTER + MAX_ENV; ++id) {
        mFreeIndices.push(id);
    }

    for (NetworkObjectIdType id{ USER_ID_START }; auto & user : mPlayers) {
        user = std::make_shared<GameObject>();
        user->InitId(id);
        user->Reset();
        user->mSpec.active = false;
        user->CreateScript<PlayerScript>(user, std::make_shared<Input>());

        ++id;
    }

    for (NetworkObjectIdType id{ MONSTER_ID_START }; auto& monster : mMonsters) {
        monster = std::make_shared<GameObject>();
        monster->InitId(id);
        monster->Reset();
        monster->mSpec.active = false;

        ++id;
    }

    for (NetworkObjectIdType id{ PROJECTILE_ID_START }; auto& projectile : mProjectiles) {
        projectile = std::make_shared<GameObject>();
        projectile->InitId(id);
        projectile->Reset();
        projectile->mSpec.active = false;

        ++id;
    }

    for (NetworkObjectIdType id{ ENV_ID_START }; auto& env : mEnvironments) {
        env = std::make_shared<GameObject>();
        env->InitId(id);
        env->Reset();
        env->mSpec.active = false;

        ++id;
    }
}

ObjectManager::~ObjectManager() { }

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
        return mMonsters[id];
    }
    else if (id < ENV_ID_START - 1) {
        return mProjectiles[id];
    }

    return mEnvironments[id];
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

    return mMonsters[id];
}

std::shared_ptr<GameObject> ObjectManager::GetEnv(NetworkObjectIdType id) const {
    if (id >= INVALID_OBJ_ID or id < ENV_ID_START) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad ENV Object Array Access - Access Id: [{}]", id);
        return nullptr;
    }

    return mPlayers[id];
}

bool ObjectManager::InViewRange(NetworkObjectIdType id1, NetworkObjectIdType id2, const float range) {
    const auto obj1 = GetObjectFromId(id1);
    const auto obj2 = GetObjectFromId(id2);

    const auto pos1 = obj1->GetPosition();
    const auto pos2 = obj2->GetPosition();

    auto dist = SimpleMath::Vector3::DistanceSquared(pos1, pos2);
    return dist < (range * range);
}
