#pragma once

void ProcessPackets(std::shared_ptr<class IServerGameScene>& gameScene, const uint8_t* buffer, size_t bufSize);
const uint8_t* ProcessPacket(std::shared_ptr<IServerGameScene>& gameScene, const uint8_t* buffer);

void ProcessPlayerLookCS(const Packets::PlayerLookCS* const look, std::shared_ptr<class GameObject>& player);
void ProcessPlayerSelectWeaponCS(const Packets::PlayerSelectWeaponCS* const weapon, std::shared_ptr<class GameObject>& player);
void ProcessPlayerSelectRoleCS(const Packets::PlayerSelectRoleCS* const roll, std::shared_ptr<class GameObject>& player);
void ProcessLatencyCS(const Packets::PacketLatencyCS* const latency);
void ProcessRequestAttackCS(const Packets::RequestAttackCS* const attack, std::shared_ptr<class GameObject>& player);
void ProcessRequestUseItemCS(const Packets::RequestUseItemCS* const useItem, std::shared_ptr<class GameObject>& player);
void ProcessRequestFireProjectileCS(const Packets::RequestFireCS* const fire, std::shared_ptr<class GameObject>& player);