#include <SFW/Connection.h>
#include <SFW/LoggerManager.h>
#include <bit>
#include <bits/stdint-uintn.h>
#include <chrono>
#include <ratio>
#include <thread>
#include <vector>

#include "BlockState.h"
#include "ClientPackets.h"
#include "PlayerHandler.h"
#include "DataTypes/BitSet.h"
#include "DataTypes/Identifier.h"
#include "DataTypes/nbt.h"
#include "Registry.h"
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
        m_context(context)
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
                    SFW_LOG_WARN("PlayerHandler", "State is unknown");
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
                SFW_LOG_DEBUG("PlayerHandler", "{}", *packet);
                switch (packet->GetNextState())
                {
                    case 2:
                        SFW_LOG_INFO("PlayerHandler", "Login request");
                        m_state = PlayerHandlerState::LOGIN;
                        break;
                    case 1:
                        SFW_LOG_INFO("PlayerHandler", "Status request");
                        m_state = PlayerHandlerState::STATUS;
                        break;
                    default:
                        SFW_LOG_WARN("PlayerHandler", "Unknown next state {}", packet->GetNextState());
                        break;
                }
                break;
            }
            default:
                SFW_LOG_WARN("PlayerHandler", "Unknown packet ID: {}", genericPacket->GetId<int>());
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
                SFW_LOG_DEBUG("PlayerHandler", "Status request sent");
                break;
            }
            case client::StatusPacketID::PING:
            {
                client::PingRequest* packet = (client::PingRequest*)genericPacket.get();
                SFW_LOG_DEBUG("PlayerHandler", "Ping request: {}", *packet);
                std::vector<uint8_t> send;
                util::writeVarInt(send, 1);
                send.resize(9);
                *(send.data() + 1) = packet->GetPayload();
                util::writeVarInt(send, 0, send.size());
                m_client.Send(send);
                break;
            }
            default:
                SFW_LOG_WARN("PlayerHandler", "Unknown packet ID: {}", genericPacket->GetId<int>());
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
                SFW_LOG_DEBUG("PlayerHandler", "{}", *packet);
                server::LoginSuccessPacket out(*packet);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                m_client.Send(out);
                SFW_LOG_DEBUG("PlayerHandler", "Success packet sent");
                break;
            }
            case client::LoginPacketID::LoginAcknowledged:
            {
                SFW_LOG_INFO("PlayerHandler", "Login Acknowledged");
                SFW_LOG_INFO("PlayerHandler", "Starting configuration");
                m_state = PlayerHandlerState::CONFIG;
                m_client.Send(server::KnownPacksPacket("minecraft", "core", "1.21.8"));
                break;
            }
            default:
                SFW_LOG_WARN("PlayerHandler", "Unknown packet ID: {}", genericPacket->GetId<int>());
                break;
        }
    }

    void PlayerHandler::OnConfig(Packet::PacketPtr&& genericPacket)
    {
        switch (genericPacket->GetId<client::ConfigPacketID>())
        {
            case client::ConfigPacketID::KnownPacks:
            {
                SFW_LOG_DEBUG("PlayerHandler", "{}", *genericPacket);

                SFW_LOG_INFO("PlayerHandler", "Sending registry data ...");
                for (const auto& registry : m_context.registry_packets)
                    m_client.Send(registry);
                SFW_LOG_INFO("PlayerHandler", "Sending registry data ... DONE");
                m_client.Send(server::FinishConfiguration());
                break;
            }
            case client::ConfigPacketID::AcknowledgeConfigEnd :
            {
                SFW_LOG_INFO("PlayerHandler", "ConfigAcknowledged switching to play state");
                m_state = PlayerHandlerState::PLAY;
                m_client.Send(server::LoginPlayPacket());
                m_client.Send(server::GameEvent(server::GameEvent::Event::StartWaitingForChunks, 0));

                break;
            }
            default:
            {
                SFW_LOG_WARN("PlayerHandler", "Unknown packet ID: {}", genericPacket->GetId<int>());
                break;
            }
        }
    }

    void PlayerHandler::OnPlay(Packet::PacketPtr&& genericPacket)
    {
        switch (genericPacket->GetId<client::PlayPacketID>())
        {

            default:
                SFW_LOG_WARN("PlayerHandler", "Unknown packet ID:{}", genericPacket->GetId<int>());
                break;
        }
    }

    void PlayerHandler::PlayLoop()
    {
    }
}