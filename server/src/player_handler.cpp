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

#include "block_state.hpp"
#include "client_packets.hpp"
#include "player_handler.hpp"
#include "data_types/bitset.hpp"
#include "data_types/identifier.hpp"
#include "data_types/nbt.hpp"
#include "packet_dispatcher.hpp"
#include "registry.hpp"
#include "server_context.hpp"
#include "server_packets.hpp"
#include "server_state.hpp"
#include "utils.hpp"
#include "packet.hpp"

namespace mc
{
    player_handler::player_handler(iu::Connection& client, const ServerContext& context, packet_dispatcher& dispatcher, server_state& state)
        : m_client(client),
        m_state(state),
        m_context(context)
    {
        register_callbacks(dispatcher);
    }

    void player_handler::play_loop()
    {

    }

    void player_handler::register_callbacks(packet_dispatcher& dispatcher)
    {
        /* LOGIN */
        dispatcher.register_callback(
            client::login_packet_id::start,
            bind_callback(&player_handler::on_login_start)
        );
        dispatcher.register_callback(
            client::login_packet_id::login_ack,
            bind_callback(&player_handler::on_login_ack)
        );

        /* CONFIG */
        dispatcher.register_callback(
            client::config_packet_id::known_packs,
            bind_callback(&player_handler::on_known_packs)
        );
        dispatcher.register_callback(
            client::config_packet_id::ack_config_end,
            bind_callback(&player_handler::on_ack_config_end)
        );
    }

    /************
    * CALLBACKS *
    ************/

    /********
    * LOGIN *
    *********/

    coro::task<void> player_handler::on_login_start(client::login_start_packet& start_packet)
    {
        server::login_success_packet success_packet{start_packet};
        static std::atomic_bool received(false);
        if (!received)
        {
            received = true;
            m_client.Send(success_packet);
            SFW_LOG_DEBUG("player_handler", "Success packet sent");
        }

        co_return;
    }

    coro::task<void> player_handler::on_login_ack([[maybe_unused]]client::login_ack& ack_packet)
    {
        SFW_LOG_INFO("player_handler", "Login Acknowledged");
        SFW_LOG_INFO("player_handler", "Starting configuration");
        m_state.set_state(state::config);
        m_client.Send(server::known_packs("minecraft", "core", "1.21.8"));

        SFW_LOG_INFO("player_handler", "Sending registry data ...");

        for (const auto& registry : m_context.registry_packets)
            m_client.Send(registry);

        SFW_LOG_INFO("player_handler", "Sending registry data ... DONE");
        m_client.Send(server::finish_config{});
        co_return;
}

/************
* CONFIGURE *
************/

    coro::task<void> player_handler::on_known_packs(client::known_packs_packet& known_packs)
    {
        SFW_LOG_DEBUG("player_handler", "{}", known_packs);
        co_return;

    }

    coro::task<void> player_handler::on_ack_config_end([[maybe_unused]]client::ack_config& config_ack)
    {

        SFW_LOG_INFO("PlayerHandler", "ConfigAcknowledged switching to play state");
        m_state.set_state(state::play);

        m_client.Send(server::login_play_packet());

        SFW_LOG_INFO("PlayerHandler", "Login(play) sent");
        m_client.Send(server::game_event(server::game_event::event::StartWaitingForChunks, 0));
        SFW_LOG_INFO("PlayerHandler", "GameEvent with StartWaitingForChunks sent");

        {
            std::vector<uint8_t> chunk_center = {3, 0x57,1 ,1};
            m_client.Send(chunk_center);
        }

        const auto& chunk = m_context.chunk_region[0][0].value();
        const auto& world_surface = chunk["Heightmaps"]["WORLD_SURFACE"].get_ref<nbt::LongArray>();
        const auto& motion_blocking = chunk["Heightmaps"]["MOTION_BLOCKING"].get_ref<nbt::LongArray>();
        const auto& motion_blocking_no_leaves = chunk["Heightmaps"]["MOTION_BLOCKING_NO_LEAVES"].get_ref<nbt::LongArray>();
        const auto& sections = chunk["sections"].get_ref<nbt::list>();

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
            util::writeVarInt(chunk_data, world_surface.size());
            iu::Serializer<std::vector<nbt::Long>>().Serialize(chunk_data, world_surface);

            //motion_blocking map
            //type of map
            util::writeVarInt(chunk_data, 4);
            util::writeVarInt(chunk_data, motion_blocking.size());
            iu::Serializer<std::vector<nbt::Long>>().Serialize(chunk_data, motion_blocking);

            //motion blocking no leaves map
            //type of map
            util::writeVarInt(chunk_data, 5);
            util::writeVarInt(chunk_data, motion_blocking_no_leaves.size());
            iu::Serializer<std::vector<nbt::Long>>().Serialize(chunk_data, motion_blocking_no_leaves);

            std::vector<uint8_t> chunk_section_data;
            for ([[maybe_unused]] const auto& section : sections)
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

            bit_set sky_light_mask(sections.size() + 2);
            bit_set block_light_mask(sections.size() + 2);
            bit_set empty_sky_light_mask(sections.size()+ 2);
            bit_set empty_block_light_mask(sections.size() + 2);

            for (size_t idx : std::views::iota(0zu, sections.size() + 2))
            {
                sky_light_mask.set(idx, true);
                block_light_mask.set(idx, true);
                empty_sky_light_mask.set(idx, false);
                empty_block_light_mask.set(idx, false);
            }

            BitSetSerializer().Serialize(chunk_data, sky_light_mask);
            BitSetSerializer().Serialize(chunk_data, block_light_mask);
            BitSetSerializer().Serialize(chunk_data, empty_sky_light_mask);
            BitSetSerializer().Serialize(chunk_data, empty_block_light_mask);

            util::writeVarInt(chunk_data, sections.size() + 2);
            for ([[maybe_unused]]size_t idx : std::views::iota(0zu, sections.size() + 2))
            {
                util::writeVarInt(chunk_data, 2048);
                iu::Serializer<std::vector<uint8_t>>().Serialize(chunk_data, std::ranges::to<std::vector<uint8_t>>(std::views::repeat(0xee, 2048)));
            }

            util::writeVarInt(chunk_data, sections.size() + 2);
            for ([[maybe_unused]]size_t idx : std::views::iota(0zu, sections.size() + 2))
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
            m_client.Send(server::sync_player_position(0, 30, 320, 30, 0, 0, 0, 0, 0, 0));
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

        co_return;
    }

}
