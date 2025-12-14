#ifndef PLAYER_HANDLER_HPP
#define PLAYER_HANDLER_HPP
#include <concepts>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <SFW/Connection.h>

#include "packet.hpp"
#include "client_packets.hpp"
#include "packet_dispatcher.hpp"
#include "server_context.hpp"
#include "server_packets.hpp"
#include "server_state.hpp"

namespace mc
{
    class player_handler
    {
    public:
        player_handler() = delete;
        player_handler(const player_handler&) = delete;
        player_handler(player_handler&&) = delete;

        player_handler& operator=(const player_handler&) = delete;
        player_handler& operator=(player_handler&&) = delete;

        player_handler(iu::Connection& client, const ServerContext& context, packet_dispatcher& dispatcher, server_state& state);
        ~player_handler() = default;

        //void Execute(const std::vector<uint8_t>& data);

        void play_loop();

    private:

        template<typename T>
            requires std::derived_from<T, packet>
        std::function<void(T&)> bind_callback(void (player_handler::*method)(T&))
        {
            return [this, method](T& packet) { std::invoke(method, this, packet); };
        }

        void register_callbacks(packet_dispatcher& dispatcher);

        /************
        * CALLBACKS *
        *************/

        void on_login_start(client::login_start_packet& start_packet);
        void on_login_ack(client::login_ack& ack_packet);

        void on_known_packs(client::known_packs_packet& known_packs);
        void on_ack_config_end(client::ack_config& conifg_ack);

        iu::Connection& m_client;
        server_state m_state;
        const ServerContext& m_context;
    };
}
#endif //PLAYER_HANDLER_H
