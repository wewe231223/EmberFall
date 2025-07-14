#pragma once

class GameSession;

void ProcessPackets(GameSession* session, const uint8_t* const buffer, size_t bufSize);
const uint8_t* ProcessPacket(GameSession* session, const uint8_t* buffer);

void ProcessHeartBeatCS(GameSession* session, const Packets::HeartBeatCS* const heartbeat);
void ProcessPlayerEnterInLobby(GameSession* session, const Packets::PlayerEnterInLobbyCS* const enter);
void ProcessPlayerReadyInLobby(GameSession* session, const Packets::PlayerReadyInLobbyCS* const ready);
void ProcessPlayerCancelReady(GameSession* session, const Packets::PlayerCancelReadyCS* const cencelReady);
void ProcessPlayerExitCS(GameSession* session, const Packets::PlayerExitCS* const exit);
void ProcessPlayerEnterInGame(GameSession* session, const Packets::PlayerEnterInGame* const enter);
void ProcessPlayerInputCS(GameSession* session, const Packets::PlayerInputCS* const input);
void ProcessPlayerLookCS(GameSession* session, const Packets::PlayerLookCS* const look);
void ProcessPlayerSelectRoleCS(GameSession* session, const Packets::PlayerSelectRoleCS* const roll);
void ProcessLatencyCS(GameSession* session, const Packets::PacketLatencyCS* const latency);
void ProcessRequestAttackCS(GameSession* session, const Packets::RequestAttackCS* const attack);
void ProcessRequestUseItemCS(GameSession* session, const Packets::RequestUseItemCS* const useItem);
void ProcessRequestFireProjectileCS(GameSession* session, const Packets::RequestFireCS* const fire);

void ProcessTestChangeToNextSceneCS(GameSession* session);