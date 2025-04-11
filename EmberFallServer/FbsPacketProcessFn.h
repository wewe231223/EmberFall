#pragma once

void ProcessPackets(std::shared_ptr<class IServerGameScene>& gameScene, const uint8_t* const buffer, size_t bufSize);
const uint8_t* ProcessPacket(std::shared_ptr<IServerGameScene>& gameScene, const uint8_t* buffer);

void ProcessPlayerInputCS(const Packets::PlayerInputCS* const input, std::shared_ptr<class GameObject>& player);
void ProcessPlayerLookCS(const Packets::PlayerLookCS* const look, std::shared_ptr<class GameObject>& player);
void ProcessPlayerSelectWeaponCS(const Packets::PlayerSelectWeaponCS* const weapon, std::shared_ptr<class GameObject>& player);
void ProcessPlayerSelectRoleCS(const Packets::PlayerSelectRoleCS* const roll, std::shared_ptr<class GameObject>& player);
void ProcessLatencyCS(const Packets::PacketLatencyCS* const latency);
void ProcessRequestAttackCS(const Packets::RequestAttackCS* const attack, std::shared_ptr<class GameObject>& player);
void ProcessRequestUseItemCS(const Packets::RequestUseItemCS* const useItem, std::shared_ptr<class GameObject>& player);
void ProcessRequestFireProjectileCS(const Packets::RequestFireCS* const fire, std::shared_ptr<class GameObject>& player);

void ProcessPackets(std::shared_ptr<class GameSession>& session, const uint8_t* const buffer, size_t bufSize);
const uint8_t* ProcessPacket(std::shared_ptr<class GameSession>& session, const uint8_t* buffer);
void ProcessPlayerInputCS(std::shared_ptr<class GameSession>& session, const Packets::PlayerInputCS* const input);
void ProcessPlayerLookCS(std::shared_ptr<class GameSession>& session, const Packets::PlayerLookCS* const look);
void ProcessPlayerSelectWeaponCS(std::shared_ptr<class GameSession>& session, const Packets::PlayerSelectWeaponCS* const weapon);
void ProcessPlayerSelectRoleCS(std::shared_ptr<class GameSession>& session, const Packets::PlayerSelectRoleCS* const roll);
void ProcessLatencyCS(std::shared_ptr<class GameSession>& session, const Packets::PacketLatencyCS* const latency);
void ProcessRequestAttackCS(std::shared_ptr<class GameSession>& session, const Packets::RequestAttackCS* const attack);
void ProcessRequestUseItemCS(std::shared_ptr<class GameSession>& session, const Packets::RequestUseItemCS* const useItem);
void ProcessRequestFireProjectileCS(std::shared_ptr<class GameSession>& session, const Packets::RequestFireCS* const fire);