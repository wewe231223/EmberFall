#include "pch.h"
#include "FbsPacketFactory.h"
#include "Session.h"

const PacketHeaderSC* FbsPacketFactory::GetHeaderPtrSC(const uint8_t* const data) {
    return reinterpret_cast<const PacketHeaderSC*>(data);
}

const PacketHeaderCS* FbsPacketFactory::GetHeaderPtrCS(const uint8_t* const data) {
    return reinterpret_cast<const PacketHeaderCS*>(data);
}

OverlappedSend* FbsPacketFactory::ClonePacket(OverlappedSend* const overlapped) {
    auto copyOverlapped = mSendPacketBuffers->GetOverlapped(overlapped);
    return copyOverlapped;
}

void FbsPacketFactory::ReleasePacketBuf(OverlappedSend* const overlapped) {
    if (true == mSendPacketBuffers->ReleaseOverlapped(overlapped)) {
        return;
    }
    gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Release Send Overlapped Failure!");
}

OverlappedSend* FbsPacketFactory::ProtocolVersionSC() {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateProtocolVersionSC(builder);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::NotifyIdSC(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateNotifyIdSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_NOTIFY_ID_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerExitSC(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerExitSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_PLAYER_EXIT_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PacketLatencySC(uint64_t time) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePacketLatencySC(builder, time);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_LATENCT_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerEnterInLobbySC(SessionIdType id, uint8_t slotIndex, Packets::PlayerRole role, std::string_view name) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto nameOffset = builder.CreateString(name.data());
    auto offset = Packets::CreatePlayerEnterInLobbySC(builder, id, slotIndex, role, nameOffset);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_PLAYER_ENTER_IN_LOBBY_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerReadyInLobbySC(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerReadyInLobbySC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_PLAYER_READY_IN_LOBBY_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerCancelReadySC(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerCancelReadySC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_PLAYER_CANCEL_READY_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::RejectSelectionRoleSC() {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateRejectSelectionRoleSC(builder);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_REJECT_SELECTION_ROLE_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ConfirmSelectoinRoleSC() {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateConfirmSelectionRoleSC(builder);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_CONFIRM_SELECTION_ROLE_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerChangeRoleSC(SessionIdType id, Packets::PlayerRole role) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerChangeRoleSC(builder, id, role);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_PLAYER_CHANGE_ROLE_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ChangeToNextSceneSC() {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateChangeToNextSceneSC(builder);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_CHANGE_TO_NEXT_SCENE_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::GameEndSC(Packets::PlayerRole winner) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateGameEndSC(builder, winner);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_GAME_END_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectAppearedSC(NetworkObjectIdType id, Packets::EntityType entity, float yaw,
    Packets::AnimationState animation, float hp, const SimpleMath::Vector3& pos) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsPos = GetVec3(pos);
    auto offset = Packets::CreateObjectAppearedSC(builder, id, entity, animation, hp, yaw, &fbsPos);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_OBJECT_APPEARED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectDisappearedSC(NetworkObjectIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectDisappearedSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_OBJECT_DISAPPEARED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectRemoveSC(NetworkObjectIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectRemovedSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_OBJECT_REMOVED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectMoveSC(NetworkObjectIdType id, float yaw,
    const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, float speed) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsPos = GetVec3(pos);
    Packets::Vec3 fbsDir = GetVec3(dir);
    auto offset = Packets::CreateObjectMoveSC(builder, id, yaw, &fbsPos, &fbsDir, speed);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_OBJECT_MOVE_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectAttackedSC(NetworkObjectIdType id, float hp) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectAttackedSC(builder, id, hp);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_OBJECT_ATTACKED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectAnimationChangedSC(NetworkObjectIdType id, Packets::AnimationState animation) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectAnimationChangedSC(builder, id, animation);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_OBJECT_ANIMATION_CHANGED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::UseItemSC(SessionIdType id, Packets::ItemType item) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateUseItemSC(builder, item);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_USE_ITEM_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::AcquireItemSC(SessionIdType id, Packets::ItemType item) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateAcquiredItemSC(builder, id, item);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_ACQUIRED_ITEM_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::GemInteractSC(NetworkObjectIdType objId, SessionIdType playerId) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateGemInteractSC(builder, objId, playerId);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_GEM_INTERACT_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::GemInteractionCancelSC(NetworkObjectIdType objId, SessionIdType playerId) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateGemInteractionCancelSC(builder, playerId, objId);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_GEM_CANCEL_INTERACTOIN_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::GemDestroyedSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsPos = GetVec3(pos);
    auto offset = Packets::CreateGemDestroyedSC(builder, id, &fbsPos);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_GEM_DESTROYED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::FireProjectileSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos,
    const SimpleMath::Vector3& dir, float speed, Packets::ProjectileTypes projectile) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsPos = GetVec3(pos);
    Packets::Vec3 fbsDir = GetVec3(dir);
    auto offset = Packets::CreateFireProjectileSC(builder, id, &fbsPos, &fbsDir, speed, projectile);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes_PT_FIRE_PROJECTILE_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerEnterInLobbyCS(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerEnterInLobbyCS(builder);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_PLAYER_ENTER_IN_LOBBY_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerReadyInLobbyCS(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerReadyInLobbyCS(builder);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_PLAYER_READY_IN_LOBBY_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerCancelReadyCS(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{};
    auto offset = Packets::CreatePlayerCancelReadyCS(builder);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_PLAYER_CANCEL_READY_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerEnterInGame(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerEnterInGame(builder);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_PLAYER_ENTER_INGAME, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerExitCS(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerExitCS(builder);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_PLAYER_EXIT_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerInputCS(SessionIdType id, uint8_t key, bool down) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerInputCS(builder, key, down);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_PLAYER_INPUT_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerLookCS(SessionIdType id, const SimpleMath::Vector3& look) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsLook = GetVec3(look);
    auto offset = Packets::CreatePlayerLookCS(builder, &fbsLook);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_PLAYER_LOOK_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerSelectRole(SessionIdType id, Packets::PlayerRole role) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerSelectRoleCS(builder, role);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_PLAYER_SELECT_ROLE_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::LatencyCS(SessionIdType id, uint64_t time) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePacketLatencyCS(builder, time);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_LATENCY_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::RequestUseItemCS(SessionIdType id, Packets::ItemType item) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateRequestUseItemCS(builder, item);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());    

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_REQUEST_USE_ITEM_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::RequestAttackCS(SessionIdType id, const SimpleMath::Vector3& dir) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsDir = GetVec3(dir);
    auto offset = Packets::CreateRequestAttackCS(builder, &fbsDir);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_REQUEST_ATTACK_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::RequestFireCS(SessionIdType id, const SimpleMath::Vector3& dir, Packets::ProjectileTypes projectile) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsDir = GetVec3(dir);
    auto offset = Packets::CreateRequestFireCS(builder, &fbsDir, projectile);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderCS headerCS{ sizeof(PacketHeaderCS) + payloadSize, Packets::PacketTypes_PT_REQUEST_FIRE_CS, id };
    return mSendPacketBuffers->GetOverlapped(&headerCS, payload, payloadSize);
}

Packets::Vec3 FbsPacketFactory::GetVec3(const SimpleMath::Vector3& vec) {
    return Packets::Vec3{ vec.x, vec.y, vec.z };
}

Packets::Vec2 FbsPacketFactory::GetVec2(const SimpleMath::Vector2& vec) {
    return Packets::Vec2{ vec.x, vec.y };
}

Packets::Vec2 FbsPacketFactory::GetVec2(const SimpleMath::Vector3& vec) {
    return Packets::Vec2{ vec.x, vec.z };
}

SimpleMath::Vector3 FbsPacketFactory::GetVector3(const Packets::Vec3* vec) {
    return SimpleMath::Vector3{ vec->x(), vec->y(), vec->z() };
}

SimpleMath::Vector2 FbsPacketFactory::GetVector2(const Packets::Vec2* vec) {
    return SimpleMath::Vector2{ vec->x(), vec->y() };
}

SimpleMath::Vector2 FbsPacketFactory::GetVector2(const Packets::Vec3* vec) {
    return SimpleMath::Vector2{ vec->x(), vec->z() };
}
