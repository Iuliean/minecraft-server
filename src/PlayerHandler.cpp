#include <SFW/Connection.h>
#include <SFW/LoggerManager.h>
#include <bits/stdint-uintn.h>
#include <vector>

#include "ClientPackets.h"
#include "PlayerHandler.h"
#include "DataTypes/Identifier.h"
#include "SFW/Serializer.h"
#include "ServerContext.h"
#include "ServerPackets.h"
#include "utils.h"
#include "Packet.h"

namespace mc
{
    PlayerHandler::PlayerHandler(iu::Connection& client, const ServerContext& context)
        : m_client(client),
        m_state(PlayerHandlerState::IDLE),
        m_context(context),
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
                case PlayerHandlerState::CONFIG:
                    packet = NextPacketConfig(packetsIter);
                    if(packet == nullptr)
                        return;
                    OnConfig(std::move(packet));
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
        switch (genericPacket->GetId<client::StatusPacketID>())
        {
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
        switch (genericPacket->GetId<client::LoginPacketID>())
        {
            case client::LoginPacketID::START:
            {
                client::LoginStartPacket* packet = (client::LoginStartPacket*) genericPacket.get();
                m_logger.debug("{}", *packet);
                server::LoginSuccessPacket out(*packet);
                m_client.Send(out);
                break;
            }
            case client::LoginPacketID::LoginAcknowledged:
            {
                m_logger.debug("Login Acknowledged");
                m_logger.info("Switching to config state");
                m_state = PlayerHandlerState::CONFIG;
                //std::vector<std::uint8_t> out{0x07};
                //iu::Serializer<Identifier>().Serialize(out, Identifier("lol"));
                //util::writeVarInt(out, 0);
                //util::writeVarInt(out, 0, out.size());
                //m_client.Send(out);
                
                m_client.Send(m_context.registry_packets[0]);
                m_client.Send(m_context.registry_packets[1]);
                m_client.Send(m_context.registry_packets[2]);
                m_client.Send(m_context.registry_packets[3]);
                m_client.Send(m_context.registry_packets[4]);
                m_client.Send(std::vector<std::uint8_t>{0x01,0x03});
                break;
            }
            default:
                m_logger.warn("Unknown packet ID: {}", genericPacket->GetId<int>());
                break;
        }
    }

    void PlayerHandler::OnConfig(Packet::PacketPtr&& genericPacket)
    {
        switch (genericPacket->GetId<client::ConfigPacketID>())
        {
            case client::ConfigPacketID::AcknowledgeConfigEnd :
            {
                m_logger.info("ConfigAcknowledged switching to play state");
                m_state = PlayerHandlerState::PLAY;
                m_client.Send(server::LoginPlayPacket());
                m_client.Send(std::vector<std::uint8_t>{0x06, 0x22, 0x01, 0x00, 0x00, 0x00, 0x00});
                std::vector<std::uint8_t> out;
                out.push_back(0x40);
                util::DoubleSerializer().Serialize(out, 100);
                util::DoubleSerializer().Serialize(out, 100);
                util::DoubleSerializer().Serialize(out, 100);
                util::FloatSerializer().Serialize(out, 100);
                util::FloatSerializer().Serialize(out, 50);
                util::ByteSerializer().Serialize(out, 0);
                util::writeVarInt(out, 0);
                util::writeVarInt(out, 0, out.size());
                m_client.Send(out);
                break;
            }
            default:
            {
                m_logger.warn("Unknown packet ID: {}", genericPacket->GetId<int>());
                break;
            }
        }
    }

    void PlayerHandler::OnPlay(Packet::PacketPtr&& genericPacket)
    {
        switch (genericPacket->GetId<client::PlayPacketID>())
        {

            default:
                m_logger.warn("Unknown packet ID:{}", genericPacket->GetId<int>());
                break;
        }
    }

    void PlayerHandler::PlayLoop()
    {
    }
}