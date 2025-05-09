#pragma once

void ProcessPackets(std::shared_ptr<class GameSession>& session, const uint8_t* const buffer, size_t bufSize);
const uint8_t* ProcessPacket(std::shared_ptr<class GameSession>& session, const uint8_t* buffer);

void ProcessPlayerEnterInLobby(std::shared_ptr<class GameSession>& session, const Packets::PlayerEnterInLobbyCS* const enter);
void ProcessPlayerReadyInLobby(std::shared_ptr<class GameSession>& session, const Packets::PlayerReadyInLobbyCS* const ready);
void ProcessPlayerCancelReady(std::shared_ptr<class GameSession>& session, const Packets::PlayerCancelReadyCS* const cencelReady);
void ProcessPlayerExitCS(std::shared_ptr<class GameSession>& session, const Packets::PlayerExitCS* const exit);
void ProcessPlayerEnterInGame(std::shared_ptr<class GameSession>& session, const Packets::PlayerEnterInGame* const enter);
void ProcessPlayerInputCS(std::shared_ptr<class GameSession>& session, const Packets::PlayerInputCS* const input);
void ProcessPlayerLookCS(std::shared_ptr<class GameSession>& session, const Packets::PlayerLookCS* const look);
void ProcessPlayerSelectRoleCS(std::shared_ptr<class GameSession>& session, const Packets::PlayerSelectRoleCS* const roll);
void ProcessLatencyCS(std::shared_ptr<class GameSession>& session, const Packets::PacketLatencyCS* const latency);
void ProcessRequestAttackCS(std::shared_ptr<class GameSession>& session, const Packets::RequestAttackCS* const attack);
void ProcessRequestUseItemCS(std::shared_ptr<class GameSession>& session, const Packets::RequestUseItemCS* const useItem);
void ProcessRequestFireProjectileCS(std::shared_ptr<class GameSession>& session, const Packets::RequestFireCS* const fire);