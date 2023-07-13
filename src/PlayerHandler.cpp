#include <bits/stdint-uintn.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "Connection.h"
#include "PlayerHandler.h"
#include "ServerConnectionHandler.h"
#include "client/Packet.h"
#include "spdlog/common.h"
#include "utils.h"

namespace mc
{
    PlayerHandler::PlayerHandler(iu::Connection& client)
        : m_client(client),
        m_state(PlayerHandlerState::IDLE)
    {
            m_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            m_logger = std::make_shared<spdlog::logger>("PlayerHandler", m_sink);
            m_logger->set_level(spdlog::level::debug);
            m_statusMessage={
            {"version",
                {
                    {"name","1.20.1"},
                    {"protocol",763}
                }
            },
            {"players",
                {
                    {"max", 10},
                    {"online", 0},
                    {"sample",
                        {
                            {"name", "thinkofdeath"},
                            {"id", "4566e69f-c907-48ee-8d71-d7ba5aa00d20"}
                        }
                    }
                }
            },
            {"description",
                {
                    {"text", "This is a shit implementation of an Mc server that doesn't even work"}
                }
            },
            {"enforceSecureChat", true},
            {"previewsChat", true}
        };
 
    }

    void PlayerHandler::Execute(const std::vector<uint8_t>& packets)
    {
        auto packetsIter = packets.begin();
        mc::client::Packet::PacketPtr packet;
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
                    m_logger->warn("State is unknown");
                    return;
            }
        }
    }

    void PlayerHandler::OnIdle(client::Packet::PacketPtr&& genericPacket)
    {
        switch((client::IdlePacketID)genericPacket->GetId())
        {
            case client::IdlePacketID::HANDSHAKE:
            {
                mc::client::HandshakePacket* packet = (mc::client::HandshakePacket*)genericPacket.get();  
                m_logger->debug("{}", *packet);
                switch (packet->GetNextState())
                {
                    case 2:
                        m_logger->info("Login request");
                        m_state = PlayerHandlerState::LOGIN;
                        break;
                    case 1:
                        m_logger->info("Status request");
                        m_state = PlayerHandlerState::STATUS;
                        break;
                    default:
                        m_logger->warn("Unknown next state {}", packet->GetNextState());
                        break;    
                }
                break;
            }
            default:
                m_logger->warn("Unknown packet ID: {}", genericPacket->GetId());
                break;
        }
    }

    void PlayerHandler::OnStatus(client::Packet::PacketPtr&& genericPacket)
    {
        switch ((client::StatusPacketID) genericPacket->GetId()) {
            case client::StatusPacketID::STATUS:
            {
                std::vector<uint8_t> send;
                util::writeVarInt(send, 0);
                util::writeStringToBuff(send, m_statusMessage.dump());
                util::writeVarInt(send, 0, send.size());
                m_client.SendAll(send);
                m_logger->debug("Status request");
                break;
            }
            case client::StatusPacketID::PING:
            {
                client::PingRequest* packet = (client::PingRequest*)genericPacket.get();
                m_logger->debug("Ping request: {}", *packet);
                std::vector<uint8_t> send;
                util::writeVarInt(send, 1);
                send.resize(9);
                *(send.data() + 1) = packet->GetPayload();
                util::writeVarInt(send, 0, send.size());
                m_client.SendAll(send);
                break;
            }
            default:
                m_logger->warn("Unknown packet ID: {}", genericPacket->GetId());
                break;
        }
    }

    void PlayerHandler::OnLogin(client::Packet::PacketPtr&& genericPacket)
    {
        switch ((client::LoginPacketID) genericPacket->GetId()) {
            case client::LoginPacketID::START:
            {
                client::LoginStartPacket* packet = (client::LoginStartPacket*) genericPacket.get();
                m_logger->debug("{}", *packet);
                break;
            }
            default:
                m_logger->warn("Unknown packet ID: {}", genericPacket->GetId());
                break;
        }
    }

    void PlayerHandler::OnPlay(client::Packet::PacketPtr&& packet)
    {
        m_logger->debug("OnPlay");
    }
}