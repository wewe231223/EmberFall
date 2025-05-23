#include "pch.h"
#include "ObjectManager.h"

#include "PlayerScript.h"
#include "MonsterScript.h"
#include "CorruptedGem.h"
#include "Trigger.h"
#include "EventTrigger.h"
#include "ItemScript.h"

#include "Input.h"
#include "Sector.h"

#include "Resources.h"

#include "GameRoom.h"

ObjectManager::ObjectManager(uint16_t roomIdx) 
    : mRoomIdx{ roomIdx } { }

ObjectManager::~ObjectManager() { }

void ObjectManager::SetSector(std::shared_ptr<SectorSystem> sector) {
    mSector = sector;
}

void ObjectManager::Init(uint16_t roomIdx) {
    for (NetworkObjectIdType id{ USER_ID_START }; auto& user : mPlayers) {
        user = std::make_shared<GameObject>(roomIdx);
        user->InitId(id);
        user->Reset();
        user->mSpec.active = false;

        ++id;
    }

    for (NetworkObjectIdType id{ MONSTER_ID_START }; auto & monster : mNPCs) {
        monster = std::make_shared<GameObject>(roomIdx);
        monster->InitId(id);
        monster->Reset();
        monster->mSpec.active = false;

        ++id;
    }

    for (NetworkObjectIdType id{ PROJECTILE_ID_START }; auto & projectile : mProjectiles) {
        projectile = std::make_shared<GameObject>(roomIdx);
        projectile->InitId(id);
        projectile->Reset();
        projectile->mSpec.active = false;

        ++id;
    }

    for (NetworkObjectIdType id{ TRIGGER_ID_START }; auto & trigger : mTriggers) {
        trigger = std::make_shared<GameObject>(roomIdx);
        trigger->InitId(id);
        trigger->Reset();
        trigger->mSpec.active = false;

        ++id;
    }

    for (NetworkObjectIdType id{ ENV_ID_START }; auto & env : mEnvironments) {
        env = std::make_shared<GameObject>(roomIdx);
        env->InitId(id);
        env->Reset();
        env->mSpec.active = false;

        ++id;
    }

}

void ObjectManager::Start(uint8_t corruptedGemCount) {
    for (uint8_t i = 0; i < corruptedGemCount; ++i) {
        SpawnObject(Packets::EntityType_CORRUPTED_GEM);
    }
}

void ObjectManager::Reset() {
    for (auto& player : mPlayers) {
        if (false == player->mSpec.active) {
            continue;
        }

        player->Reset();
    }

    for (auto& npc : mNPCs) {
        if (false == npc->mSpec.active) {
            continue;
        }

        npc->Reset();
    }

    for (auto& trigger : mTriggers) {
        if (false == trigger->mSpec.active) {
            continue;
        }

        trigger->Reset();
    }
}

void ObjectManager::LoadEnvFromFile(const std::filesystem::path& path) {
    std::ifstream envs{ path, std::ios::binary };
    if (not envs.is_open()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Load Environments Failure - File does not exists");
        return;
    }

    uint32_t numOfEnvs{ };
    envs.read(reinterpret_cast<char*>(&numOfEnvs), sizeof(numOfEnvs));
    struct FileFormat {
        GameProtocol::EnvironmentType type{ };
        SimpleMath::Vector3 pos{ };
        float yaw{ };
    };

    auto sector = mSector.lock();
    if (nullptr == sector) {
        return;
    }

    std::vector<FileFormat> envInfos(numOfEnvs);
    envs.read(reinterpret_cast<char*>(envInfos.data()), sizeof(FileFormat) * envInfos.size());
    for (auto& info : envInfos) {
        if (GameProtocol::EnvironmentType::Fern == info.type or GameProtocol::EnvironmentType::LogHouseDoor == info.type or GameProtocol::EnvironmentType::WindMillBlade == info.type) {
            continue;
        }

        auto obj = SpawnObject(Packets::EntityType_ENV);
        obj->SetTag(ObjectTag::ENV);
        obj->mSpec.entity = Packets::EntityType_ENV;
        obj->mSpec.active = true;
        obj->Init();

        obj->CreateBoundingObject<OBBCollider>(ResourceManager::GetEnvInfo(info.type).bb);

        auto objTransform = obj->GetTransform();
        objTransform->Translate(info.pos);
        objTransform->SetY(0.0f);
        objTransform->Rotation(SimpleMath::Quaternion::CreateFromYawPitchRoll(SimpleMath::Vector3{ 0.0f, info.yaw, 0.0f }));
        objTransform->Update();
        obj->GetBoundingObject()->Update(objTransform->GetWorld());

        sector->AddInSector(obj->GetId(), obj->GetPosition());
    }
}

std::shared_ptr<GameObject> ObjectManager::GetObjectFromId(NetworkObjectIdType id) const {
    if (id > VALID_ID_MAX or INVALID_SESSION_ID == id) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Object Array Access - Access Id: [{}]", id);
        return nullptr;
    }

    if (id < MONSTER_ID_START) {
        return mPlayers[id];
    }
    else if (id < PROJECTILE_ID_START) {
        return mNPCs[id - MONSTER_ID_START];
    }
    else if (id < TRIGGER_ID_START) {
        return mProjectiles[id - PROJECTILE_ID_START];
    }
    else if (id < ENV_ID_START) {
        return mTriggers[id - TRIGGER_ID_START];
    }

    return mEnvironments[id - ENV_ID_START];
}

std::shared_ptr<GameObject> ObjectManager::GetPlayer(NetworkObjectIdType id) const {
    if (id >= MONSTER_ID_START) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Player Object Array Access - Access Id: [{}]", id);
        return nullptr;
    }

    return mPlayers[id];
}

std::shared_ptr<GameObject> ObjectManager::GetNPC(NetworkObjectIdType id) const {
    if (id >= PROJECTILE_ID_START or id < MONSTER_ID_START) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Monster Object Array Access - Access Id: [{}]", id);
        return nullptr;
    }

    return mNPCs[id - MONSTER_ID_START];
}

std::shared_ptr<GameObject> ObjectManager::GetTrigger(NetworkObjectIdType id) const {
    if (id >= ENV_ID_START or id < TRIGGER_ID_START) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Trigger Object Array Access - Access Id: [{}]", id);
        return nullptr;
    }

    return mTriggers[id - TRIGGER_ID_START];
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
    if (nullptr == obj1 or nullptr == obj2) {
        return false;
    }

    const auto pos1 = obj1->GetPosition();
    const auto pos2 = obj2->GetPosition();

    auto dist = SimpleMath::Vector3::DistanceSquared(pos1, pos2);
    return dist < MathUtil::Square(range);
}

std::shared_ptr<GameObject> ObjectManager::SpawnObject(Packets::EntityType entity) {
    auto sector = mSector.lock();
    if (nullptr == sector) {
        return nullptr;
    }

    auto collisionManager = gGameRoomManager->GetRoom(mRoomIdx)->GetStage().GetCollisionManager();
    if (nullptr == collisionManager) {
        return nullptr;
    }

    NetworkObjectIdType validId{ };
    switch (entity) {
    case Packets::EntityType_MONSTER:
    {
        if (false == mNPCIndices.try_pop(validId)) {
            gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Max NPC");
            break;
        }

        auto obj = GetObjectFromId(validId);
        obj->mSpec.active = true;
        obj->CreateScript<MonsterScript>(obj);
        obj->CreateBoundingObject<OBBCollider>(ResourceManager::GetEntityInfo(ENTITY_KEY_IMP).bb);
        obj->Init();

        obj->GetTransform()->SetY(0.0f);
        auto [min, max] = GameProtocol::Logic::MONSTER_SPAWN_AREA;
        obj->GetTransform()->Translate(Random::GetRandomVec3(min, max));
        
        sector->AddInSector(validId, obj->GetPosition());

        obj->RegisterUpdate();

        return obj;
    }

    case Packets::EntityType_PROJECTILE:
    {
        break;
    }

    case Packets::EntityType_CORRUPTED_GEM:
    {
        if (false == mNPCIndices.try_pop(validId)) {
            gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Max NPC");
            break;
        }
        
        auto obj = GetObjectFromId(validId);
        obj->mSpec.active = true;
        obj->CreateScript<CorruptedGemScript>(obj);
        obj->CreateBoundingObject<OBBCollider>(ResourceManager::GetEntityInfo(ENTITY_KEY_HUMAN).bb);
        obj->Init();

        obj->GetTransform()->SetY(0.0f);
        auto [min, max] = GameProtocol::Logic::GEM_SPAWN_AREA;
        obj->GetTransform()->Translate(Random::GetRandomVec3(min, max));

        sector->AddInSector(validId, obj->GetPosition());
        obj->RegisterUpdate();

        return obj;
    }

    case Packets::EntityType_BOSS:
    {

        break;
    }

    case Packets::EntityType_ENV:
    {
        if (false == mEnvironmentsIndices.try_pop(validId)) {
            gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Max ENV");
            break;
        }

        auto obj = GetEnv(validId);
        return obj;
    }

    case Packets::EntityType_ITEM_POTION:
    {
        if (false == mNPCIndices.try_pop(validId)) {
            gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Max NPC");
            break;
        }

        auto obj = GetObjectFromId(validId);
        obj->mSpec.active = true;
        obj->CreateScript<ItemScript>(obj, ItemTag::ITEM_POTION);
        obj->CreateBoundingObject<OBBCollider>(ResourceManager::GetEntityInfo(ENTITY_KEY_HUMAN).bb);
        obj->Init();

        obj->GetTransform()->SetY(0.0f);
        auto [min, max] = GameProtocol::Logic::ITEM_SPAWN_AREA;
        obj->GetTransform()->Translate(Random::GetRandomVec3(min, max));

        sector->AddInSector(validId, obj->GetPosition());
        obj->RegisterUpdate();
        return obj;
    }

    case Packets::EntityType_ITEM_CROSS:
    {
        if (false == mNPCIndices.try_pop(validId)) {
            gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Max NPC");
            break;
        }

        auto obj = GetObjectFromId(validId);
        obj->mSpec.active = true;
        obj->CreateScript<ItemScript>(obj, ItemTag::ITEM_CROSS);
        obj->CreateBoundingObject<OBBCollider>(ResourceManager::GetEntityInfo(ENTITY_KEY_HUMAN).bb);
        obj->Init();

        obj->GetTransform()->SetY(0.0f);
        obj->GetTransform()->Translate(Random::GetRandomVec3(SimpleMath::Vector3{ -10.0f, 0.0f, -10.0f }, SimpleMath::Vector3{ 10.0f, 0.0f, 10.0f }));

        sector->AddInSector(validId, obj->GetPosition());
        obj->RegisterUpdate();
        return obj;
    }

    default:
        break;
    }

    return nullptr;
}

std::shared_ptr<GameObject> ObjectManager::SpawnTrigger(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& ext, const SimpleMath::Vector3& dir, float lifeTime) {
    auto sector = mSector.lock();
    if (nullptr == sector) {
        return nullptr;
    }

    NetworkObjectIdType validId{ };
    if (false == mTriggerIndices.try_pop(validId)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Max Trigger");
        return nullptr;
    }

    auto obj = GetTrigger(validId);
    obj->mSpec.active = true;
    obj->CreateScript<Trigger>(obj, lifeTime);
    obj->GetTransform()->SetPosition(pos);
    obj->GetTransform()->SetLook(dir);
    obj->GetTransform()->Update();

    obj->CreateBoundingObject<OBBCollider>(SimpleMath::Vector3::Zero, ext);
    
    obj->Init();

    obj->GetBoundingObject()->Update(obj->GetTransform()->GetWorld());
    sector->AddInSector(validId, obj->GetPosition());

    return obj;
}

std::shared_ptr<GameObject> ObjectManager::SpawnEventTrigger(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& ext, const SimpleMath::Vector3& dir, 
    float lifeTime, std::shared_ptr<GameEvent> event, float delay, int32_t count) {
    auto sector = mSector.lock();
    if (nullptr == sector) {
        return nullptr;
    }

    NetworkObjectIdType validId{ };
    if (false == mTriggerIndices.try_pop(validId)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Max Event Trigger");
        return nullptr;
    }

    auto obj = GetTrigger(validId);
    obj->mSpec.active = true;
    obj->CreateScript<EventTrigger>(obj, event, lifeTime, delay, count);
    obj->GetTransform()->SetPosition(pos);
    obj->GetTransform()->SetLook(dir);
    obj->GetTransform()->Update();

    obj->CreateBoundingObject<OBBCollider>(SimpleMath::Vector3::Zero, ext);

    obj->Init();

    obj->GetBoundingObject()->Update(obj->GetTransform()->GetWorld());
    obj->RegisterUpdate();

    return obj;
}

void ObjectManager::StartUpdateNPCs() {
    for (auto& npc : mNPCs) {
        if (npc->mSpec.active) {
            npc->RegisterUpdate();
        }
    }
}

void ObjectManager::ReleaseObject(NetworkObjectIdType id) {
    if (id > VALID_ID_MAX) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Bad Object Array Access - Access Id: [{}]", id);
        return;
    }

    if (id < MONSTER_ID_START) {
#if defined(DEBUG) || defined(_DEBUG)
        //gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Release Player Object");
#endif
    }
    else if (id < PROJECTILE_ID_START) {
        mNPCIndices.push(id);
    }
    else if (id < TRIGGER_ID_START) {
        mProjectileIndices.push(id);
    }
    else if (id < ENV_ID_START) {
        mTriggerIndices.push(id);
    }
    else {
        mEnvironmentsIndices.push(id);
    }
}
