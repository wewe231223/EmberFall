#pragma once

#include "../Protocol/flatbuffers/flatbuffers.h"
#include "../Protocol/PacketProtocol_generated.h"

class FbsPacketFactory {
public:
    // Server To Client
    static void ProtocolVersionSC();
    static void NotifyIdSC(SessionIdType id);
    static void PlayerExitSC(SessionIdType id);
    static void PacketLatencySC(SessionIdType id, uint64_t time);

    static void ObjectAppearedSC(NetworkObjectIdType id, AnimationState animation, float hp, const SimpleMath::Vector3& pos);
    static void ObjectDisappearedSC(NetworkObjectIdType id);
    static void ObjectRemoveSC(NetworkObjectIdType id);
    static void ObjectMoveSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, float speed);
    static void ObjectAttackedSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos, float hp);
    static void ObjectAnimationChangeddSC(NetworkObjectIdType id, AnimationState animation);

    static void UseItemSC(SessionIdType id, Packets::ItemType item);
    static void AcquireItemSC(SessionIdType id, Packets::ItemType item);

    static void GemInteractSC(NetworkObjectIdType objId, SessionIdType playerId);
    static void GemInteractionCancelSC(NetworkObjectIdType objId, SessionIdType playerId);
    static void GemDestroyedSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos);

    static void FireProjectileSC(NetworkObjectIdType id, const SimpleMath::Vector3& pos, 
        const SimpleMath::Vector3& dir, float speed, Packets::ProjectileTypes projectile);

    // Client to Server
    static void PlayerExitCS(SessionIdType id);
    static void PlayerLookCS(const SimpleMath::Vector3& look);
    static void PlayerSelectWeapon(Packets::Weapon weapon);
    static void LatencyCS(uint64_t time);

    static void RequestUseItemCS(Packets::ItemType item);

    static void RequestAttackCS(const SimpleMath::Vector3& dir);
    static void RequestFireCS(const SimpleMath::Vector3& dir, Packets::ProjectileTypes projectile);

public:
    // Process PacketFn
       // Server To Client
    static void ProcessProtocolVersionSC();
    static void ProcessNotifyIdSC(Packets::PacketHeaderSC* header, SessionIdType id);
    static void ProcessPlayerExitSC(Packets::PacketHeaderSC* header, SessionIdType id);
    static void ProcessPacketLatencySC(Packets::PacketHeaderSC* header, SessionIdType id, uint64_t time);
           
    static void ProcessObjectAppearedSC(Packets::PacketHeaderSC* header, NetworkObjectIdType id, AnimationState animation, float hp, const SimpleMath::Vector3& pos);
    static void ProcessObjectDisappearedSC(Packets::PacketHeaderSC* header, NetworkObjectIdType id);
    static void ProcessObjectRemoveSC(Packets::PacketHeaderSC* header, NetworkObjectIdType id);
    static void ProcessObjectMoveSC(Packets::PacketHeaderSC* header, NetworkObjectIdType id, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, float speed);
    static void ProcessObjectAttackedSC(Packets::PacketHeaderSC* header, NetworkObjectIdType id, const SimpleMath::Vector3& pos, float hp);
    static void ProcessObjectAnimationChangeddSC(Packets::PacketHeaderSC* header, NetworkObjectIdType id, AnimationState animation);
           
    static void ProcessUseItemSC(Packets::PacketHeaderSC* header, SessionIdType id, Packets::ItemType item);
    static void ProcessAcquireItemSC(Packets::PacketHeaderSC* header, SessionIdType id, Packets::ItemType item);
           
    static void ProcessGemInteractSC(Packets::PacketHeaderSC* header, NetworkObjectIdType objId, SessionIdType playerId);
    static void ProcessGemInteractionCancelSC(Packets::PacketHeaderSC* header, NetworkObjectIdType objId, SessionIdType playerId);
    static void ProcessGemDestroyedSC(Packets::PacketHeaderSC* header, NetworkObjectIdType id, const SimpleMath::Vector3& pos);
           
    static void ProcessFireProjectileSC(Packets::PacketHeaderSC* header, NetworkObjectIdType id, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, float speed, Packets::ProjectileTypes projectile);
           
    // Clievoid
    static void ProcessPlayerExitCS(Packets::PacketHeaderCS* header, SessionIdType id);
    static void ProcessPlayerLookCS(Packets::PacketHeaderCS* header, const SimpleMath::Vector3& look);
    static void ProcessPlayerSelectWeapon(Packets::Weapon weapon);
    static void ProcessLatencyCS(Packets::PacketHeaderCS* header, uint64_t time);
           
    static void ProcessRequestUseItemCS(Packets::PacketHeaderCS* header, Packets::ItemType item);
           
    static void ProcessRequestAttackCS(Packets::PacketHeaderCS* header, const SimpleMath::Vector3& dir);
    static void ProcessRequestFireCS(Packets::PacketHeaderCS* header, const SimpleMath::Vector3& dir, Packets::ProjectileTypes projectile);


private:
    Packets::Vec3 GetVec3(const SimpleMath::Vector3& vec);
    Packets::Vec2 GetVec2(const SimpleMath::Vector2& vec);
    Packets::Vec2 GetVec2(const SimpleMath::Vector3& vec);
};