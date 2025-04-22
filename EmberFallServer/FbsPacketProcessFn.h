#pragma once

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