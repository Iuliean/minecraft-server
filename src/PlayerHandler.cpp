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
                m_client.Send(
                    server::SynchronisePlayerPosition(-100, -100 , -100, 100, 50)
                );

                std::vector<uint8_t> out;
                std::vector<uint8_t> chunk_data;
                const NBT::NBT& chunk = m_context.chunk_region[0][0];
                out.push_back(0x27);
                util::IntSerializer().Serialize(out, chunk->Get<NBT::Int>("xPos"));
                util::IntSerializer().Serialize(out, chunk->Get<NBT::Int>("zPos"));
                SFW_LOG_INFO("PlayerHandler", "{} {}", chunk->Get<NBT::Int>("xPos"), chunk->Get<NBT::Int>("zPos"));
                NBT::NBTSerializer(true).Serialize(out, chunk->Get<NBT::NBTCompound>("Heightmaps"));
                const auto& sections = chunk->Get<NBT::NBTList>("sections");

                for (auto it = sections->begin(); it != sections->end(); it++)
                {
                    [[maybe_unused]]
                    const auto& section = it.get<NBT::NBTCompound>();
                    util::ShortSerializer().Serialize(chunk_data, 23);

                    util::ByteSerializer().Serialize(chunk_data, 15);
                    const long coarse_dirt = 0b0000000000000001011000000000001011000000000001011000000000001011;
                    const long biome = 0b0000000001000001000001000001000001000001000001000001000001000001;

                    std::vector<long> block_data_array(1024 , coarse_dirt);
                    std::vector<long> biomes_data_array(7, biome);
 

                    util::writeVarInt(chunk_data, block_data_array.size());
                    iu::Serializer<std::vector<long>>().Serialize(chunk_data, block_data_array);
                    util::ByteSerializer().Serialize(chunk_data, 6);
                    util::writeVarInt(chunk_data, biomes_data_array.size());
                    iu::Serializer<std::vector<long>>().Serialize(chunk_data, biomes_data_array);

                }
                util::writeVarInt(out, chunk_data.size());
                out.insert(out.end(), chunk_data.begin(), chunk_data.end());
                util::writeVarInt(out, 0);
                BitSetSerializer().Serialize(out, BitSet(1));
                BitSetSerializer().Serialize(out, BitSet(1));
                BitSetSerializer().Serialize(out, BitSet(1));
                BitSetSerializer().Serialize(out, BitSet(1));
                util::writeVarInt(out, 0);
                util::writeVarInt(out, 0);

                util::writeVarInt(out, 0 , out.size());
                SFW_LOG_INFO("PlayerHandler", "{:x} {:x} {}", out[0], out[1], out.size());
                m_client.Send(out);
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