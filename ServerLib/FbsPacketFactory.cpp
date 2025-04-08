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

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerExitSC(SessionIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePlayerExitSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PacketLatencySC(uint64_t time) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreatePacketLatencySC(builder, time);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
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

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectDisappearedSC(NetworkObjectIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectDisappearedSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectRemoveSC(NetworkObjectIdType id) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectRemovedSC(builder, id);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
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

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectAttackedSC(NetworkObjectIdType id, float hp) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectAttackedSC(builder, id, hp);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::ObjectAnimationChangeddSC(NetworkObjectIdType id, AnimationState animation) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateObjectAnimationChangedSC(builder, animation);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::UseItemSC(SessionIdType id, Packets::ItemType item) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateUseItemSC(builder, item);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::AcquireItemSC(SessionIdType id, Packets::ItemType item) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateAcquiredItemSC(builder, id, item);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::GemInteractSC(NetworkObjectIdType objId, SessionIdType playerId) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateGemInteractSC(builder, objId, playerId);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::GemInteractionCancelSC(NetworkObjectIdType objId, SessionIdType playerId) {
    flatbuffers::FlatBufferBuilder builder{ };

    auto offset = Packets::CreateGemInteractionCancelSC(builder, objId, playerId);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::GemDestroyedSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos) {
    flatbuffers::FlatBufferBuilder builder{ };

    Packets::Vec3 fbsPos = GetVec3(pos);
    auto offset = Packets::CreateGemDestroyedSC(builder, id, &fbsPos);
    builder.Finish(offset);

    const uint8_t* payload = builder.GetBufferPointer();
    const PacketSizeT payloadSize = static_cast<PacketSizeT>(builder.GetSize());

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
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

    PacketHeaderSC headerSC{ sizeof(PacketHeaderSC) + payloadSize, Packets::PacketTypes::PacketTypes_PT_PROTOCOL_VERSION_SC };
    return mSendPacketBuffers->GetOverlapped(&headerSC, payload, payloadSize);
}

OverlappedSend* FbsPacketFactory::PlayerExitCS(SessionIdType id) {
    return nullptr;
}

OverlappedSend* FbsPacketFactory::PlayerLookCS(const SimpleMath::Vector3& look) {
    return nullptr;
}

OverlappedSend* FbsPacketFactory::PlayerSelectWeapon(Packets::Weapon weapon) {
    return nullptr;
}

OverlappedSend* FbsPacketFactory::LatencyCS(uint64_t time)
{
    return nullptr;
}

OverlappedSend* FbsPacketFactory::RequestUseItemCS(Packets::ItemType item)
{
    return nullptr;
}

OverlappedSend* FbsPacketFactory::RequestAttackCS(const SimpleMath::Vector3& dir) {
    return nullptr;
}

OverlappedSend* FbsPacketFactory::RequestFireCS(const SimpleMath::Vector3& dir, Packets::ProjectileTypes projectile) {
    return nullptr;
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
