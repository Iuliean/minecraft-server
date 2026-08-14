#ifndef MINECRAFT_HANDLER_HPP
#define MINECRAFT_HANDLER_HPP
#include <SFW/ServerConnectionHandler.h>
#include <SFW/Connection.h>

#include <atomic>

#include "client_packets.hpp"
#include "player_handler.hpp"
#include "server_context.hpp"
#include "server_state.hpp"
#include "packet_dispatcher.hpp"
#include "packet.hpp"
#include "utils.hpp"

namespace mc
{

    class minecraft_handler: public iu::ServerConnectionHandler, packet_dispatcher
    {
    public:
        minecraft_handler();
        virtual ~minecraft_handler()= default;

        void HandleConnection(iu::Connection& connection)override;
        void OnConnected(iu::Connection& connection)override;
        void Stop()override;

    private:
        void register_callbacks();

        void dispatch(packet_ptr packet) override;

        void build_registry_pakcets();

        /***********
         * PARSING *
         **********/

        template<util::IteratorU8 Iter>
        packet_ptr parse_packet(Iter& iter)
        {
            switch (m_state.get_state())
            {
                using enum state;
                case idle: return client::parse_idle_packet(iter);
                case status: return client::parse_status_packet(iter);
                case config: return client::parse_config_packet(iter);
                case login: return client::parse_login_packet(iter);
                case play: return client::parse_play_packet(iter);
                default:
                {
                    SFW_LOG_ERROR("minecraft_handler", "HUH!!!?!?!?!?!?!");
                    throw std::runtime_error("LOLLOLOL");
                }
            }
        }

        /************
        * CALLBACKS *
        ************/

        template<typename T>
            requires std::derived_from<T, packet>
        std::function<coro::task<void>(T&)> bind_callback(coro::task<void> (minecraft_handler::*method)(T&))
        {
            return [this, method](T& packet) { return std::invoke(method, this, packet); };
        }

        /*******
        * IDLE *
        *******/
        coro::task<void> on_handshake(client::handshake_packet& handshake);
        coro::task<void> on_status(client::status_request_packet& status);
        coro::task<void> on_ping(client::ping_request& ping);

        /********
        * LOGIN *
        ********/
        coro::task<void> on_login_start(client::login_start_packet& start_packet);

        server_state m_state;
        ServerContext m_context;
        std::optional<iu::Connection> m_client;
        std::atomic_bool m_stop;
    };
}

#endif //MINECRAFT_HANDLER_HPP
