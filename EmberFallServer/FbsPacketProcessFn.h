#pragma once
#include "ServerGameScene.h"

inline void ProcessPackets(std::shared_ptr<IServerGameScene>& gameScene, const uint8_t* buffer, size_t bufSize);

inline const uint8_t* ProcessPacket(std::shared_ptr<IServerGameScene>& gameScene, const uint8_t* buffer);

inline void ProcessPlayerLookCS(const Packets::PlayerLookCS* const look, std::shared_ptr<GameObject>& player);

inline void ProcessPlayerSelectWeaponCS(const Packets::PlayerSelectWeaponCS* const weapon, std::shared_ptr<GameObject>& player);

inline void ProcessPlayerSelectRoleCS(const Packets::PlayerSelectRoleCS* const roll, std::shared_ptr<GameObject>& player);

inline void ProcessLatencyCS(const Packets::PacketLatencyCS* const latency);

inline void ProcessRequestAttackCS(const Packets::RequestAttackCS* const attack, std::shared_ptr<GameObject>& player);

inline void ProcessRequestUseItemCS(const Packets::RequestUseItemCS* const useItem, std::shared_ptr<GameObject>& player);

inline void ProcessRequestFireProjectileCS(const Packets::RequestFireCS* const fire, std::shared_ptr<GameObject>& player);