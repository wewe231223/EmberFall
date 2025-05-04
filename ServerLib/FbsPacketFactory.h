#pragma once

#include "../Protocol/flatbuffers/flatbuffers.h"
#include "../Protocol/PacketProtocol_generated.h"
#include "SendBuffers.h"

inline constexpr size_t FBS_PACKET_CS_START = Packets::PacketTypes_MIN;
inline constexpr size_t FBS_PACKET_CS_END = Packets::PacketTypes_PT_REQUEST_FIRE_CS;
inline constexpr size_t FBS_PACKET_SC_START = Packets::PacketTypes_PT_PROTOCOL_VERSION_SC;
inline constexpr size_t FBS_PACKET_SC_END = Packets::PacketTypes_MAX;

class FbsPacketFactory {
public:
    static const PacketHeaderSC* GetHeaderPtrSC(const uint8_t* const data);
    static const PacketHeaderCS* GetHeaderPtrCS(const uint8_t* const data);

    template <typename PacketType>
    static const PacketType* GetDataPtrSC(const uint8_t* const data) {
        auto header = GetHeaderPtrSC(data);
        return flatbuffers::GetRoot<PacketType>(data + sizeof(PacketHeaderSC));
    }

    template <typename PacketType>
    static const PacketType* GetDataPtrCS(const uint8_t* const data) {
        auto header = GetHeaderPtrCS(data);
        return flatbuffers::GetRoot<PacketType>(data + sizeof(PacketHeaderCS));
    }

public:
    static OverlappedSend* ClonePacket(OverlappedSend* const overlapped);
    static void ReleasePacketBuf(OverlappedSend* const overlapped);

public:
    // Server To Client
    static OverlappedSend* ProtocolVersionSC();
    static OverlappedSend* NotifyIdSC(SessionIdType id);
    static OverlappedSend* PlayerExitSC(SessionIdType id);
    static OverlappedSend* PacketLatencySC(uint64_t time);

    // In Lobby
    static OverlappedSend* PlayerEnterInLobbySC(SessionIdType id, uint8_t slotIndex, Packets::PlayerRole role, std::string_view name);
    static OverlappedSend* PlayerReadyInLobbySC(SessionIdType id);
    static OverlappedSend* PlayerCancelReadySC(SessionIdType id);
    static OverlappedSend* RejectSelectionRoleSC();
    static OverlappedSend* ConfirmSelectoinRoleSC();
    static OverlappedSend* PlayerChangeRoleSC(SessionIdType id, Packets::PlayerRole role);
    static OverlappedSend* ChangeToNextSceneSC();
    static OverlappedSend* GameEndSC(Packets::PlayerRole winner);

    static OverlappedSend* ObjectAppearedSC(NetworkObjectIdType id, Packets::EntityType entity, float yaw,
        Packets::AnimationState animation, float hp, const SimpleMath::Vector3& pos);
    static OverlappedSend* ObjectDisappearedSC(NetworkObjectIdType id);
    static OverlappedSend* ObjectRemoveSC(NetworkObjectIdType id);
    static OverlappedSend* ObjectMoveSC(NetworkObjectIdType id, float yaw, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, float speed);
    static OverlappedSend* ObjectAttackedSC(NetworkObjectIdType id, float hp);
    static OverlappedSend* ObjectAnimationChangedSC(NetworkObjectIdType id, Packets::AnimationState animation);

    static OverlappedSend* UseItemSC(SessionIdType id, Packets::ItemType item);
    static OverlappedSend* AcquireItemSC(SessionIdType id, Packets::ItemType item);

    static OverlappedSend* GemInteractSC(NetworkObjectIdType objId, SessionIdType playerId);
    static OverlappedSend* GemInteractionCancelSC(NetworkObjectIdType objId, SessionIdType playerId);
    static OverlappedSend* GemDestroyedSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos);

    static OverlappedSend* FireProjectileSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos,
        const SimpleMath::Vector3& dir, float speed, Packets::ProjectileTypes projectile);

    // Client to Server
    // In Lobby
    static OverlappedSend* PlayerEnterInLobbyCS(SessionIdType id);
    static OverlappedSend* PlayerReadyInLobbyCS(SessionIdType id);
    static OverlappedSend* PlayerCancelReadyCS(SessionIdType id);

    // InGame
    static OverlappedSend* PlayerEnterInGame(SessionIdType id);
    static OverlappedSend* PlayerExitCS(SessionIdType id);
    static OverlappedSend* PlayerInputCS(SessionIdType id, uint8_t key, bool down);
    static OverlappedSend* PlayerLookCS(SessionIdType id, const SimpleMath::Vector3& look);
    static OverlappedSend* PlayerSelectRole(SessionIdType id, Packets::PlayerRole role);
    static OverlappedSend* LatencyCS(SessionIdType id, uint64_t time);
           
    static OverlappedSend* RequestUseItemCS(SessionIdType id, Packets::ItemType item);
           
    static OverlappedSend* RequestAttackCS(SessionIdType id, const SimpleMath::Vector3& dir);
    static OverlappedSend* RequestFireCS(SessionIdType id, const SimpleMath::Vector3& dir, Packets::ProjectileTypes projectile);

public:
    static Packets::Vec3 GetVec3(const SimpleMath::Vector3& vec);
    static Packets::Vec2 GetVec2(const SimpleMath::Vector2& vec);
    static Packets::Vec2 GetVec2(const SimpleMath::Vector3& vec);

    static SimpleMath::Vector3 GetVector3(const Packets::Vec3* vec);
    static SimpleMath::Vector2 GetVector2(const Packets::Vec2* vec);
    static SimpleMath::Vector2 GetVector2(const Packets::Vec3* vec);

private:
    inline static std::shared_ptr<SendBufferFactory> mSendPacketBuffers{ std::make_shared<SendBufferFactory>() };
};