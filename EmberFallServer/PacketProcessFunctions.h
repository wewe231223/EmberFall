#pragma once

#include "ServerGameScene.h"
#include "GameObject.h"
#include "Input.h"

inline void ProcessPacketKeyInput(PacketHeader* header, const std::shared_ptr<InputManager>& inputManager)
{
    auto keyInput = reinterpret_cast<PacketCS::PacketKeyInput*>(header);
    auto input = inputManager->GetInput(keyInput->id);

    input->UpdateInput(keyInput->key, keyInput->down);
}

inline void ProcessPacketCamera(PacketHeader* header, PlayerMap& players)
{
    auto cameraPacket = reinterpret_cast<PacketCS::PacketCamera*>(header);

    if (players.contains(cameraPacket->id)) {
        players[cameraPacket->id]->GetTransform()->SetLook(cameraPacket->look);
    }
}

inline void ProcessPacketExit(PacketHeader* header, std::shared_ptr<SessionManager>& sessionManager)
{
    sessionManager->CloseSession(header->id);

    PacketSC::PacketPlayerExit exitPacket{ sizeof(PacketSC::PacketPlayerExit), PacketType::PACKET_PLAYER_EXIT, header->id };
    sessionManager->SendAll(&exitPacket);
}

inline void ProcessPacketAttack(PacketHeader* header, PlayerMap& players)
{
    auto attackPacket = reinterpret_cast<PacketCS::PacketRequestAttack*>(header);
    players[attackPacket->id]->Attack();
}

inline void ProcessPacketSelectRoll(PacketHeader* header, PlayerMap& players)
{
    // Todo... ?
    auto selectRoll = reinterpret_cast<PacketCS::PacketSelectRole*>(header);
}

inline void ProcessPacketSelectWeapon(PacketHeader* header, PlayerMap& players)
{
    auto selectWeapon = reinterpret_cast<PacketCS::PacketSelectWeapon*>(header);
    players[selectWeapon->id]->ChangeWeapon(selectWeapon->weapon);
}
