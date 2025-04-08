#pragma once

#include "../Protocol/flatbuffers/flatbuffers.h"
#include "../Protocol/PacketProtocol_generated.h"
#include "SendBuffers.h"

class FbsPacketFactory {
public:
    static void ReleasePacketBuf(OverlappedSend* const overlapped);

public:
    // Server To Client
    static OverlappedSend* ProtocolVersionSC();
    static OverlappedSend* NotifyIdSC(SessionIdType id);
    static OverlappedSend* PlayerExitSC(SessionIdType id);
    static OverlappedSend* PacketLatencySC(uint64_t time);

    static OverlappedSend* ObjectAppearedSC(NetworkObjectIdType id, Packets::EntityType entity,
        Packets::AnimationState animation, float hp, const SimpleMath::Vector3& pos);
    static OverlappedSend* ObjectDisappearedSC(NetworkObjectIdType id);
    static OverlappedSend* ObjectRemoveSC(NetworkObjectIdType id);
    static OverlappedSend* ObjectMoveSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, float speed);
    static OverlappedSend* ObjectAttackedSC(NetworkObjectIdType id, float hp);
    static OverlappedSend* ObjectAnimationChangeddSC(NetworkObjectIdType id, AnimationState animation);

    static OverlappedSend* UseItemSC(SessionIdType id, Packets::ItemType item);
    static OverlappedSend* AcquireItemSC(SessionIdType id, Packets::ItemType item);

    static OverlappedSend* GemInteractSC(NetworkObjectIdType objId, SessionIdType playerId);
    static OverlappedSend* GemInteractionCancelSC(NetworkObjectIdType objId, SessionIdType playerId);
    static OverlappedSend* GemDestroyedSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos);

    static OverlappedSend* FireProjectileSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos,
        const SimpleMath::Vector3& dir, float speed, Packets::ProjectileTypes projectile);

    // Client to Server
    static OverlappedSend* PlayerExitCS(SessionIdType id);
    static OverlappedSend* PlayerLookCS(const SimpleMath::Vector3& look);
    static OverlappedSend* PlayerSelectWeapon(Packets::Weapon weapon);
    static OverlappedSend* LatencyCS(uint64_t time);
           
    static OverlappedSend* RequestUseItemCS(Packets::ItemType item);
           
    static OverlappedSend* RequestAttackCS(const SimpleMath::Vector3& dir);
    static OverlappedSend* RequestFireCS(const SimpleMath::Vector3& dir, Packets::ProjectileTypes projectile);

public:
    //// Process PacketFn
    //   // Server To Client
    //static void ProcessProtocolVersionSC();
    //static void ProcessNotifyIdSC(const PacketHeaderSC* header, SessionIdType id);
    //static void ProcessPlayerExitSC(const PacketHeaderSC* header, SessionIdType id);
    //static void ProcessPacketLatencySC(const PacketHeaderSC* header, SessionIdType id, uint64_t time);

    //static void ProcessObjectAppearedSC(const PacketHeaderSC* header, NetworkObjectIdType id, AnimationState animation, float hp, const SimpleMath::Vector3& pos);
    //static void ProcessObjectDisappearedSC(const PacketHeaderSC* header, NetworkObjectIdType id);
    //static void ProcessObjectRemoveSC(const PacketHeaderSC* header, NetworkObjectIdType id);
    //static void ProcessObjectMoveSC(const PacketHeaderSC* header, NetworkObjectIdType id, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, float speed);
    //static void ProcessObjectAttackedSC(const PacketHeaderSC* header, NetworkObjectIdType id, const SimpleMath::Vector3& pos, float hp);
    //static void ProcessObjectAnimationChangeddSC(const PacketHeaderSC* header, NetworkObjectIdType id, AnimationState animation);

    //static void ProcessUseItemSC(const PacketHeaderSC* header, SessionIdType id, Packets::ItemType item);
    //static void ProcessAcquireItemSC(const PacketHeaderSC* header, SessionIdType id, Packets::ItemType item);

    //static void ProcessGemInteractSC(const PacketHeaderSC* header, NetworkObjectIdType objId, SessionIdType playerId);
    //static void ProcessGemInteractionCancelSC(const PacketHeaderSC* header, NetworkObjectIdType objId, SessionIdType playerId);
    //static void ProcessGemDestroyedSC(const PacketHeaderSC* header, NetworkObjectIdType id, const SimpleMath::Vector3& pos);

    //static void ProcessFireProjectileSC(const PacketHeaderSC* header, NetworkObjectIdType id, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, float speed, Packets::ProjectileTypes projectile);

    //// Clievoid
    //static void ProcessPlayerExitCS(const PacketHeaderCS* header, SessionIdType id);
    //static void ProcessPlayerLookCS(const PacketHeaderCS* header, const SimpleMath::Vector3& look);
    //static void ProcessPlayerSelectWeapon(Packets::Weapon weapon);
    //static void ProcessLatencyCS(const PacketHeaderCS* header, uint64_t time);

    //static void ProcessRequestUseItemCS(const PacketHeaderCS* header, Packets::ItemType item);

    //static void ProcessRequestAttackCS(const PacketHeaderCS* header, const SimpleMath::Vector3& dir);
    //static void ProcessRequestFireCS(const PacketHeaderCS* header, const SimpleMath::Vector3& dir, Packets::ProjectileTypes projectile);


private:
    static Packets::Vec3 GetVec3(const SimpleMath::Vector3& vec);
    static Packets::Vec2 GetVec2(const SimpleMath::Vector2& vec);
    static Packets::Vec2 GetVec2(const SimpleMath::Vector3& vec);

private:
    inline static std::shared_ptr<SendBufferFactory> mSendPacketBuffers{ std::make_shared<SendBufferFactory>() };
};