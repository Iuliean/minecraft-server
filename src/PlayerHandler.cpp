#include <bits/stdint-uintn.h>

#include "ClientPackets.h"
#include "Connection.h"
#include "PlayerHandler.h"
#include "ServerPackets.h"
#include "LoggerManager.h"
#include "utils.h"
#include "Packet.h"

namespace mc
{
    PlayerHandler::PlayerHandler(iu::Connection& client)
        : m_client(client),
        m_state(PlayerHandlerState::IDLE),
        m_logger(iu::LoggerManager::GetLogger("PlayerHandler"))
    { 
    }

    void PlayerHandler::Execute(const std::vector<uint8_t>& packets)
    {
        auto packetsIter = packets.begin();
        Packet::PacketPtr packet;
        
        while(true)
        {
            switch(m_state)
            {
                case PlayerHandlerState::IDLE:
                    packet = NextPacketIdle(packetsIter);
                    if(packet == nullptr)
                        return;
                    OnIdle(std::move(packet));
                    break;
                case PlayerHandlerState::STATUS:
                    packet = NextPacketStatus(packetsIter);
                    if(packet == nullptr)
                        return;
                    OnStatus(std::move(packet));
                    break;
                case PlayerHandlerState::LOGIN:
                    packet = NextPacketLogin(packetsIter);
                    if(packet == nullptr)
                        return;
                    OnLogin(std::move(packet));
                    break;
                case PlayerHandlerState::PLAY:
                    packet = NextPacketPlay(packetsIter);
                    if(packet == nullptr)
                        return;
                    OnPlay(std::move(packet));
                    break;
                default:
                    m_logger.warn("State is unknown");
                    return;
            }
        }
    
    }

    void PlayerHandler::OnIdle(Packet::PacketPtr&& genericPacket)
    {
        switch(genericPacket->GetId<client::IdlePacketID>())
        {
            case client::IdlePacketID::HANDSHAKE:
            {
                mc::client::HandshakePacket* packet = (mc::client::HandshakePacket*)genericPacket.get();  
                m_logger.debug("{}", *packet);
                switch (packet->GetNextState())
                {
                    case 2:
                        m_logger.info("Login request");
                        m_state = PlayerHandlerState::LOGIN;
                        break;
                    case 1:
                        m_logger.info("Status request");
                        m_state = PlayerHandlerState::STATUS;
                        break;
                    default:
                        m_logger.warn("Unknown next state {}", packet->GetNextState());
                        break;    
                }
                break;
            }
            default:
                m_logger.warn("Unknown packet ID: {}", genericPacket->GetId<int>());
                break;
        }
    }

    void PlayerHandler::OnStatus(Packet::PacketPtr&& genericPacket)
    {
        switch (genericPacket->GetId<client::StatusPacketID>()) {
            case client::StatusPacketID::STATUS:
            {
                m_client.Send(m_statusMessage);
                m_logger.debug("Status request sent");
                break;
            }
            case client::StatusPacketID::PING:
            {
                client::PingRequest* packet = (client::PingRequest*)genericPacket.get();
                m_logger.debug("Ping request: {}", *packet);
                std::vector<uint8_t> send;
                util::writeVarInt(send, 1);
                send.resize(9);
                *(send.data() + 1) = packet->GetPayload();
                util::writeVarInt(send, 0, send.size());
                m_client.SendAll(send);
                break;
            }
            default:
                m_logger.warn("Unknown packet ID: {}", genericPacket->GetId<int>());
                break;
        }
    }

    void PlayerHandler::OnLogin(Packet::PacketPtr&& genericPacket)
    {
        switch (genericPacket->GetId<client::LoginPacketID>()) {
            case client::LoginPacketID::START:
            {
                client::LoginStartPacket* packet = (client::LoginStartPacket*) genericPacket.get();
                m_logger.debug("{}", *packet);
                server::LoginSuccessPacket out(*packet);
                m_client.Send(out);
                m_state = PlayerHandlerState::PLAY;
                break;
            }
            default:
                m_logger.warn("Unknown packet ID: {}", genericPacket->GetId<int>());
                break;
        }
    }

    void PlayerHandler::OnPlay(Packet::PacketPtr&& genericPacket)
    {
        switch (genericPacket->GetId<client::PlayPacketID>())
        {
            case client::PlayPacketID::LoginAcknowledged:
            {
                m_logger.debug("Login Acknowledged");
                break;
            }
            default:
                m_logger.warn("Unknown packet ID:{}", genericPacket->GetId<int>());
                break;
        }
    }

    void PlayerHandler::PlayLoop()
    {
    }
}