#include <SFW/Connection.h>
#include <SFW/LoggerManager.h>
#include <bit>
#include <bits/stdint-uintn.h>
#include <chrono>
#include <ranges>
#include <ratio>
#include <thread>
#include <algorithm>
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
                SFW_LOG_INFO("PlayerHandler", "Login(play) sent");
                m_client.Send(server::GameEvent(server::GameEvent::Event::StartWaitingForChunks, 0));
                SFW_LOG_INFO("PlayerHandler", "GameEvent with StartWaitingForChunks sent");

                {
                    std::vector<uint8_t> chunk_center = {3, 0x57,1 ,1};
                    m_client.Send(chunk_center);
                }

                const auto& chunk = m_context.chunk_region[0][0].value();
                const auto& world_surface = chunk->Get<NBT::NBTCompound>("Heightmaps")->Get<NBT::LongArray>("WORLD_SURFACE");
                const auto& motion_blocking = chunk->Get<NBT::NBTCompound>("Heightmaps")->Get<NBT::LongArray>("MOTION_BLOCKING");
                const auto& motion_blocking_no_leaves = chunk->Get<NBT::NBTCompound>("Heightmaps")->Get<NBT::LongArray>("MOTION_BLOCKING_NO_LEAVES");
                const auto& sections = chunk->Get<NBT::NBTList>("sections");
                for (int i : std::views::iota(0,16))
                    for(int j : std::views::iota(0,16))
                {
                    std::vector<uint8_t> chunk_data;

                    util::writeVarInt(chunk_data, 0x27); //ID
                    
                    util::IntSerializer().Serialize(chunk_data, i);
                    util::IntSerializer().Serialize(chunk_data, j);
                
                    //3 heighetmaps
                    util::writeVarInt(chunk_data, 3);
                    
                    //world surface map
                    //type of map
                    util::writeVarInt(chunk_data, 1);
                    util::writeVarInt(chunk_data, world_surface.Get().size());
                    iu::Serializer<std::vector<NBT::Long>>().Serialize(chunk_data, world_surface.Get());
                    
                    //motion_blocking map
                    //type of map
                    util::writeVarInt(chunk_data, 4);
                    util::writeVarInt(chunk_data, motion_blocking.Get().size());
                    iu::Serializer<std::vector<NBT::Long>>().Serialize(chunk_data, motion_blocking.Get());
                
                    //motion blocking no leaves map
                    //type of map
                    util::writeVarInt(chunk_data, 5);
                    util::writeVarInt(chunk_data, motion_blocking_no_leaves.Get().size());
                    iu::Serializer<std::vector<NBT::Long>>().Serialize(chunk_data, motion_blocking_no_leaves.Get());
                
                    std::vector<uint8_t> chunk_section_data;
                    for (auto it = sections->begin(); it != sections->end(); it++)
                    {
                        //non air blocks
                        util::ShortSerializer().Serialize(chunk_section_data, 10000);
                    
                        //Block states
                        //0 bpe singled value palette
                        util::writeVarInt(chunk_section_data, 0);
                        //Value of the palette
                        util::writeVarInt(chunk_section_data, 10);

                        //Biomes
                        //0 bpe singled value palette
                        util::writeVarInt(chunk_section_data, 0);
                        //Value of the palette
                        util::writeVarInt(chunk_section_data, 10);
                    }

                    util::writeVarInt(chunk_data, chunk_section_data.size());
                    iu::Serializer<decltype(chunk_section_data)>().Serialize(chunk_data, chunk_section_data);
                    //Block entities
                    util::writeVarInt(chunk_data, 0);
                
                    BitSet sky_light_mask(sections->Size() + 2);
                    BitSet block_light_mask(sections->Size() + 2);
                    BitSet empty_sky_light_mask(sections->Size()+ 2);
                    BitSet empty_block_light_mask(sections->Size() + 2);
                
                    for (size_t idx : std::views::iota(0zu, sections->Size() + 2))
                    {
                        sky_light_mask.Set(idx, true);
                        block_light_mask.Set(idx, true);
                        empty_sky_light_mask.Set(idx, false);
                        empty_block_light_mask.Set(idx, false);
                    }
                    BitSetSerializer().Serialize(chunk_data, sky_light_mask);
                    BitSetSerializer().Serialize(chunk_data, block_light_mask);
                    BitSetSerializer().Serialize(chunk_data, empty_sky_light_mask);
                    BitSetSerializer().Serialize(chunk_data, empty_block_light_mask);
                
                    util::writeVarInt(chunk_data, sections->Size() + 2);
                    for ([[maybe_unused]]size_t idx : std::views::iota(0zu, sections->Size() + 2))
                    {
                        util::writeVarInt(chunk_data, 2048);
                        iu::Serializer<std::vector<uint8_t>>().Serialize(chunk_data, std::ranges::to<std::vector<uint8_t>>(std::views::repeat(0xee, 2048)));
                    }
                
                    util::writeVarInt(chunk_data, sections->Size() + 2);
                    for ([[maybe_unused]]size_t idx : std::views::iota(0zu, sections->Size() + 2))
                    {
                        util::writeVarInt(chunk_data, 2048);
                        iu::Serializer<std::vector<uint8_t>>().Serialize(chunk_data, std::ranges::to<std::vector<uint8_t>>(std::views::repeat(0xee, 2048)));
                    }
                
                    util::writeVarInt(chunk_data, 0, chunk_data.size());
                
                    m_client.Send(chunk_data);
                    SFW_LOG_INFO("PlayerHandler", "Chunk Data Sent {} {}", i, j);
                }
                for (;;)
                {
                    SFW_LOG_INFO("PlayerHandler", "Sent sync packet");
                    m_client.Send(server::SynchronisePlayerPosition(0, 30, 320, 30, 0, 0, 0, 0, 0, 0));
                    std::this_thread::sleep_for(std::chrono::seconds(10));
                }
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