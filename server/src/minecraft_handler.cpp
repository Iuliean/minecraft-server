#include <SFW/LoggerManager.h>
#include <SFW/Connection.h>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <fstream>


#include "minecraft_handler.hpp"
#include "client_packets.hpp"
#include "packet_dispatcher.hpp"
#include "server_context.hpp"
#include "server_packets.hpp"
#include "data_types/nbt.hpp"
#include "packet.hpp"
#include "server_state.hpp"
#include "zstr.hpp"

namespace mc
{

    namespace
    {
        constexpr static int PACKET_SIZE = 1024;
        //Temporary hopefully
        [[maybe_unused]]
        ChunkRegion loadChunkRegion(std::filesystem::path path)
        {

            ChunkRegion out;
            std::ifstream data(path);
            std::array<int, 1024> chunkOffsets;
            [[maybe_unused]]
            constexpr size_t  a = 4 * ((0 & 31) + (0 & 31) * 32);
            data.read((char*)chunkOffsets.data(), sizeof(chunkOffsets));
            data.seekg(0);
            for (const int offsetAndSize : chunkOffsets)
            {
                const size_t offset = int(std::byteswap(int(offsetAndSize & 0x00ffffff)) >> 8) * 4096;
                const size_t chunk_size = (offsetAndSize & 0xff000000) >> 24;

                if (offset == 0 && chunk_size == 0) continue;

                SFW_LOG_DEBUG("chunkLoading", "Loading chunk at offset:{:#x} with length:{} * 4kb sectors", offset, chunk_size);
                data.seekg(offset + 5, data.beg);

                zstr::istreambuf decompBuff(data.rdbuf());
                std::istream stream(&decompBuff);

                nbt::nbt chunk = nbt::parse(stream);

                const int x = chunk["xPos"];
                const int z = chunk["zPos"];
                SFW_LOG_INFO("chunkLoading","Chunk x{}, z{}", x, z);

                out[x][z].emplace(std::move(chunk));
            }
            return out;
        }
    }

    minecraft_handler::minecraft_handler()
        : ServerConnectionHandler(),
          m_state(),
          m_player_handler(),
          m_client(),
          m_stop(false)
    {
        register_callbacks();
        m_state.set_state(state::idle);
        m_context.chunk_region = loadChunkRegion("map/r.0.0.mca");
    }

    void minecraft_handler::OnConnected(iu::Connection& connection)
    {
        SFW_LOG_INFO("MinecraftHandler", "New connection from: {}:{}", connection.GetAdress(), connection.GetPort());
    }

    void minecraft_handler::HandleConnection(iu::Connection &connection)
    {
        m_client.emplace(connection);
        try
        {
            std::array<uint8_t, PACKET_SIZE> data;
            packet_ptr packet{nullptr};

            while(true)
            {
                const size_t recieved_size = connection.Receive(data);
                if (recieved_size == 0)
                {
                    SFW_LOG_INFO("MinecraftHandler", "No data received from {}:{}", connection.GetAdress(), connection.GetPort());
                    return;
                }

                auto iter = data.begin();
                while (iter != data.begin() + recieved_size)
                {
                    packet = parse_packet(iter);

                    if (packet)
                        dispatch(std::move(packet));
                    else
                        break;
                }


            }

        }
        catch(const std::exception& e)
        {
            SFW_LOG_ERROR("MinecraftHandler", "Failed to handle connection: {}", e.what());
        }
    }
    /**********
    * PRIVATE *
    **********/

    void minecraft_handler::register_callbacks()
    {
        register_callback(
            client::idle_packet_id::handshake,
            bind_callback(&minecraft_handler::on_handshake)
        );
        register_callback(
            client::status_packet_id::status,
            bind_callback(&minecraft_handler::on_status)
        );
        register_callback(
            client::status_packet_id::ping,
            bind_callback(&minecraft_handler::on_ping)
        );
    }
    void minecraft_handler::dispatch(packet_ptr packet)
    {
        auto dispatch_with_error = [] <typename T> (packet_ptr packet, const map_type<T>& callback_map)
        {
            auto it = callback_map.find(packet->get_id<T>());
            auto end = callback_map.end();

            if(it == end)
            {
                SFW_LOG_ERROR("minecraft_handler", "No callback for {}", packet->get_id<int>());
                return;
            }

            it->second(std::move(packet));
        };


        switch(m_state.get_state())
        {
            using enum state;
            case idle:
            {
                dispatch_with_error(std::move(packet), m_idle_cb);
                break;
            }
            case status:
            {
                dispatch_with_error(std::move(packet), m_status_cb);
                break;
            }
            case login:
            {
                dispatch_with_error(std::move(packet), m_login_cb);
                break;
            }
            case config:
            {
                dispatch_with_error(std::move(packet), m_config_cb);
                break;
            }
            case play:
            {
                dispatch_with_error(std::move(packet), m_play_cb);
                break;
            }
        }
    }

    void minecraft_handler::Stop()
    {
        return;
    }

    /************
    * CALLBACKS *
    ************/

    void minecraft_handler::on_handshake(client::handshake_packet& handshake)
    {
        SFW_LOG_DEBUG("minecraft_handler", "{}", handshake);
        const auto next_state = state_from(handshake.get_next_state());

        if (!next_state)
        {
            SFW_LOG_ERROR("minecraft_handler", "Could not convert {} to state value", handshake.get_next_state());
            return;
        }

        switch (*next_state)
        {
            using enum state;
            case login:
            {
                SFW_LOG_INFO("minecraft_handler", "Login request");
                m_state.set_state(state::login);
                m_player_handler.emplace(
                    *m_client,
                    m_context,
                    dynamic_cast<packet_dispatcher&>(*this),
                    m_state
                );
                break;
            }
            case status:
            {
                SFW_LOG_INFO("minecraft_handler", "Status request");
                m_state.set_state(state::status);
                break;
            }
            default:
                SFW_LOG_WARN("minecraft_handler", "Cannot go from idle to {}", *next_state);
        }
    }

    void minecraft_handler::on_status(client::status_request_packet& status)
    {
        assert(m_client);
        m_client->Send(server::status_packet{});
        SFW_LOG_DEBUG("minecraft_handler", "Status request sent");
    }

    void minecraft_handler::on_ping(client::ping_request& ping)
    {
        SFW_LOG_DEBUG("minecraft_handler", "Ping request: {}", ping);
        assert(m_client);
        std::vector<uint8_t> send;
        util::writeVarInt(send, 1);
        send.resize(9);
        *(send.data() + 1) = ping.get_payload();
        util::writeVarInt(send, 0, send.size());
        m_client->Send(send);
    }
}
