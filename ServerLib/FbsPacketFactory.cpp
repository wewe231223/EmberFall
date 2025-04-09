#include "pch.h"
#include "FbsPacketFactory.h"

void FbsPacketFactory::ReleasePacketBuf(OverlappedSend* const overlapped) {
    mSendPacketBuffers->ReleaseOverlapped(overlapped);
}

OverlappedSend* FbsPacketFactory::ProtocolVersionSC() {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateProtocolVersionSC(builder);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::NotifyIdSC(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateNotifyIdSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_NOTIFY_ID_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerExitSC(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerExitSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PLAYER_EXIT_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PacketLatencySC(uint64_t time) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePacketLatencySC(builder, time);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_LATENCT_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectAppearedSC(NetworkObjectIdType id, Packets::EntityType entity,
    Packets::AnimationState animation, float hp, const SimpleMath::Vector3& pos) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsPos = GetVec3(pos);
    auto offset = Packets::CreateObjectAppearedSC(builder, id, entity, animation, hp, &fbsPos);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_OBJECT_APPEARED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectDisappearedSC(NetworkObjectIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectDisappearedSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_OBJECT_DISAPPEARED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectRemoveSC(NetworkObjectIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectRemovedSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_OBJECT_REMOVED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectMoveSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, float speed) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsPos = GetVec3(pos);
    Packets::Vec3 fbsDir = GetVec3(dir);
    auto offset = Packets::CreateObjectMoveSC(builder, id, &fbsPos, &fbsDir, speed);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_OBJECT_MOVE_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectAttackedSC(NetworkObjectIdType id, float hp) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectAttackedSC(builder, id, hp);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_OBJECT_ATTACKED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectAnimationChangedSC(NetworkObjectIdType id, AnimationState animation) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectAnimationChangedSC(builder, animation);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_OBJECT_ANIMATION_CHANGED_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::UseItemSC(SessionIdType id, Packets::ItemType item) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateUseItemSC(builder, item);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_USE_ITEM_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::AcquireItemSC(SessionIdType id, Packets::ItemType item) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateAcquiredItemSC(builder, id, item);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_ACQUIRED_ITEM_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::GemInteractSC(NetworkObjectIdType objId, SessionIdType playerId) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateGemInteractSC(builder, objId, playerId);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_GEM_INTERACT_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::GemInteractionCancelSC(NetworkObjectIdType objId, SessionIdType playerId) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateGemInteractionCancelSC(builder, playerId, objId);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_GEM_CANCEL_INTERACTOIN_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::GemDestroyedSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsPos = GetVec3(pos);
    auto offset = Packets::CreateGemDestroyedSC(builder, id, &fbsPos);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_GEM_DESTROYED_SC };
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

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_FIRE_PROJECTILE_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerExitCS(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerExitCS(builder);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PLAYER_EXIT_CS };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerLookCS(const SimpleMath::Vector3& look) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsLook = GetVec3(look);
    auto offset = Packets::CreatePlayerLookCS(builder, &fbsLook);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PLAYER_LOOK_CS };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerSelectWeapon(Packets::Weapon weapon) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerSelectWeaponCS(builder, weapon);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PLAYER_SELECT_WEAPON_CS };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::LatencyCS(uint64_t time) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePacketLatencyCS(builder, time);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_LATENCY_CS };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::RequestUseItemCS(Packets::ItemType item) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateRequestUseItemCS(builder, item);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_REQUEST_USE_ITEM_CS };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::RequestAttackCS(const SimpleMath::Vector3& dir) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsDir = GetVec3(dir);
    auto offset = Packets::CreateRequestAttackCS(builder, &fbsDir);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_REQUEST_ATTACK_CS };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::RequestFireCS(const SimpleMath::Vector3& dir, Packets::ProjectileTypes projectile) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsDir = GetVec3(dir);
    auto offset = Packets::CreateRequestFireCS(builder, &fbsDir, projectile);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_REQUEST_FIRE_CS };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
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
