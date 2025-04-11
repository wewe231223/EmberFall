#include "pch.h"
#include "ObjectManager.h"

#include "PlayerScript.h"
#include "MonsterScript.h"

#include "Input.h"

void ObjectManager::Init() {
    for (NetworkObjectIdType id = 0; id < MAX_USER + MAX_MONSTER + MAX_ENV; ++id) {
        mFreeIndices.push(id);
    }

    for (auto& user : mPlayers) {
        user = std::make_shared<GameObject>();
        user->Reset();
        user->mSpec.active = false;
        user->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.3f, 1.5f, 0.3f });
        user->CreateComponent<PlayerScript>(user, std::make_shared<Input>());
    }

    for (auto& monster : mMonsters) {
        monster = std::make_shared<GameObject>();
        monster->Reset();
        monster->mSpec.active = false;
        //monster->CreateCollider<OrientedBoxCollider>();
        monster->CreateComponent<MonsterScript>(monster);
    }

    for (auto& env : mMonsters) { 
        env = std::make_shared<GameObject>();
        env->Reset();
        env->mSpec.active = false;
    }
}

void ObjectManager::LoadEnvFromFile() { }

std::shared_ptr<GameObject> ObjectManager::GetObjectFromId(NetworkObjectIdType id) const {
    if (id > VALID_ID_MAX) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Warning! : Bad Object Array Access - Access Id: [{}]", id);
        return nullptr;
    }

    if (id < MONSTER_ID_START) {
        return mPlayers[id];
    }
    else if (id < ENV_ID_START) {
        return mMonsters[id];
    }

    return mEnvironments[id];
}
