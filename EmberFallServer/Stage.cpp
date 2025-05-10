#include "pch.h"
#include "Stage.h"

Stage::Stage(GameStage stageIdx, uint16_t roomIdx) 
    : mStage{ stageIdx }, mGameRoomIdx{ roomIdx },
    mObjectManager{ std::make_shared<ObjectManager>(mGameRoomIdx) }, mCollisionManager{ std::make_shared<CollisionManager>(mGameRoomIdx) } {
    mSectorSystem = std::make_shared<SectorSystem>(mObjectManager);
    mObjectManager->SetSector(mSectorSystem);
}

Stage::~Stage() { }

bool Stage::GetActiveState() const {
    return mActive;
}

GameStage Stage::GetStageIdx() const {
    return mStage;
}

std::shared_ptr<SectorSystem> Stage::GetSectorSystem() const {
    return mSectorSystem;
}

std::shared_ptr<ObjectManager> Stage::GetObjectManager() const {
    return mObjectManager;
}

std::shared_ptr<CollisionManager> Stage::GetCollisionManager() const {
    return mCollisionManager;
}

void Stage::AddInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos) {
    mSectorSystem->AddInSector(id, pos);
}

void Stage::RemoveInSector(NetworkObjectIdType id, const SimpleMath::Vector3& pos) {
    mSectorSystem->RemoveInSector(id, pos);
}

void Stage::UpdateCollision(const std::shared_ptr<GameObject>& obj) {
    mCollisionManager->UpdateCollision(obj, mSectorSystem, mObjectManager);
}

bool Stage::InViewRange(NetworkObjectIdType id1, NetworkObjectIdType id2, const float range) {
    return mObjectManager->InViewRange(id1, id2, range);
}

void Stage::InitObjectManager(const std::filesystem::path& path) {
    mObjectManager->Init(mGameRoomIdx);
    mObjectManager->LoadEnvFromFile(path);
}

std::vector<NetworkObjectIdType> Stage::GetNearbyPlayers(const SimpleMath::Vector3& currPos, const float range) {
    return mSectorSystem->GetNearbyPlayers(currPos, range);
}

std::shared_ptr<GameObject> Stage::GetObjectFromId(NetworkObjectIdType id) {
    return mObjectManager->GetObjectFromId(id);
}

std::shared_ptr<GameObject> Stage::GetPlayer(NetworkObjectIdType id) {
    return mObjectManager->GetPlayer(id);
}

std::shared_ptr<GameObject> Stage::GetNPC(NetworkObjectIdType id) {
    return mObjectManager->GetNPC(id);
}

std::shared_ptr<GameObject> Stage::GetTrigger(NetworkObjectIdType id) {
    return mObjectManager->GetTrigger(id);
}

//std::shared_ptr<GameObject> Stage::GetProjectile(NetworkObjectIdType id) {
//    //return mObjectManager->GetProjec(id);
//}

std::shared_ptr<GameObject> Stage::GetEnv(NetworkObjectIdType id) {
    return mObjectManager->GetEnv(id);
}

void Stage::StartStage(uint8_t playerCount) {
    mActive.exchange(true);
    mObjectManager->Start(playerCount * 2);

    for (int i = 0; i < 100; ++i) {
        auto monster = mObjectManager->SpawnObject(Packets::EntityType_MONSTER);

        if (i < 10) {
            auto item = mObjectManager->SpawnObject(Packets::EntityType_ITEM_POTION);
        }
    }

    //gServerFrame->AddTimerEvent();
}

void Stage::EndStage() {
    mActive.exchange(false);

    mObjectManager->Reset();
    mCollisionManager->Reset();

    gLogConsole->PushLog(DebugLevel::LEVEL_INFO, "GameRoom [{}]: End GameLoop", mGameRoomIdx);
}

std::shared_ptr<GameObject> Stage::SpawnObject(Packets::EntityType entity) {
    return mObjectManager->SpawnObject(entity);
}

std::shared_ptr<GameObject> Stage::SpawnTrigger(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& ext, const SimpleMath::Vector3& dir, float lifeTime) {
    return mObjectManager->SpawnTrigger(pos, ext, dir, lifeTime);
}

std::shared_ptr<GameObject> Stage::SpawnEventTrigger(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& ext, const SimpleMath::Vector3& dir,
    float lifeTime, std::shared_ptr<GameEvent> event, float delay, int32_t count) {
    return mObjectManager->SpawnEventTrigger(pos, ext, dir, lifeTime, event, delay, count);
}

void Stage::ReleaseObject(NetworkObjectIdType id) {
    mObjectManager->ReleaseObject(id);
}

Short2 Stage::UpdateSectorPos(NetworkObjectIdType id, const SimpleMath::Vector3& prevPos, const SimpleMath::Vector3& currPos) {
    return mSectorSystem->UpdateSectorPos(id, prevPos, currPos);
}

void Stage::UpdatePlayerViewList(const std::shared_ptr<GameObject>& player, const SimpleMath::Vector3 pos, const float range) {
    return mSectorSystem->UpdatePlayerViewList(player, pos, range);
}

void Stage::UpdateEntityMove(const std::shared_ptr<GameObject>& object) {
    return mSectorSystem->UpdateEntityMove(object);
}

void Stage::Clear()
{
}

