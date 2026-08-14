#ifndef PLAYER_HANDLER_HPP
#define PLAYER_HANDLER_HPP
#include <concepts>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <SFW/Connection.h>

#include "data_types/position.hpp"
#include "packet.hpp"
#include "client_packets.hpp"
#include "packet_dispatcher.hpp"
#include "server_context.hpp"
#include "server_packets.hpp"
#include "server_state.hpp"
#include "coro/coro.hpp"

namespace mc
{
    class player_handler
    {
    public:
        player_handler() = delete;
        player_handler(const player_handler&) = delete;
        player_handler(player_handler&&) = default;

        player_handler& operator=(const player_handler&) = delete;
        player_handler& operator=(player_handler&&) = delete;

        player_handler(iu::Connection& client, const ServerContext& context, packet_dispatcher& dispatcher, server_state& state);
        ~player_handler() = default;

        //void Execute(const std::vector<uint8_t>& data);

        void play_loop();

    private:

        template<typename T>
            requires std::derived_from<T, packet>
        std::function<coro::task<void>(T&)> bind_callback(coro::task<void> (player_handler::*method)(T&))
        {
            return [this, method](T& packet) { return std::invoke(method, this, packet); };
        }

        void register_callbacks(packet_dispatcher& dispatcher);

        /************
        * CALLBACKS *
        *************/

        coro::task<void> on_login_ack(client::login_ack& ack_packet);

        coro::task<void> on_known_packs(client::known_packs_packet& known_packs);
        coro::task<void> on_ack_config_end(client::ack_config& conifg_ack);

        /*******
        * PLAY *
        *******/

        coro::task<void> on_player_loaded(client::player_loaded& packet);
        coro::task<void> on_player_input(client::player_input& input);

        iu::Connection& m_client;
        server_state& m_state;
        const ServerContext& m_context;

        position m_position;
    };
}
#endif //PLAYER_HANDLER_H
