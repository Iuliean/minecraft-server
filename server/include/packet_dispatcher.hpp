#ifndef PACKET_DISPATCHER_HPP
#define PACKET_DISPATCHER_HPP

#include <concepts>
#include <functional>
#include <flat_map>
#include <type_traits>
#include "SFW/LoggerManager.h"
#include "client_packets.hpp"
#include "packet.hpp"
#include "coro/coro.hpp"
namespace mc
{
    class packet_dispatcher
    {
    public:
        template<typename T>
            requires std::derived_from<T, packet>
        using callback = std::function<coro::task<void>(T&)>;

        virtual ~packet_dispatcher() = default;

        template<typename T>
        void register_callback(client::idle_packet_id id, callback<T> callback)
        {
            m_idle_cb.emplace(id, [callback](packet_ptr packet) { return task_entry(std::move(packet), std::move(callback)); });
        }

        template<typename T>
        void register_callback(client::status_packet_id id, callback<T> callback)
        {
            m_status_cb.emplace(id, [callback](packet_ptr packet) { return task_entry(std::move(packet), std::move(callback)); });
        }

        template<typename T>
        void register_callback(client::login_packet_id id, callback<T> callback)
        {
            m_login_cb.emplace(id, [callback](packet_ptr packet){ return task_entry(std::move(packet), std::move(callback)); });
        }

        template<typename T>
        void register_callback(client::config_packet_id id, callback<T> callback)
        {
            m_config_cb.emplace(id, [callback](packet_ptr packet){ return task_entry(std::move(packet), std::move(callback)); });
        }

        template<typename T>
        void register_callback(client::play_packet_id id, callback<T> callback)
        {
            m_play_cb.emplace(id, [callback](packet_ptr packet){ return task_entry(std::move(packet), std::move(callback)); });
        }
    private:

        template <typename T>
        static coro::task<void> task_entry(packet_ptr packet, callback<T> callback)
        {
            co_await callback(dynamic_cast<T&>(*packet));
            co_return;
        }

    protected:

        using wrapper_callback = std::function<coro::task<void>(packet_ptr)>;

        virtual void dispatch(packet_ptr packet) = 0;

        template<PacketID T>
        using map_type = std::flat_map<T, wrapper_callback>;

        map_type<client::idle_packet_id> m_idle_cb;
        map_type<client::status_packet_id> m_status_cb;
        map_type<client::login_packet_id> m_login_cb;
        map_type<client::config_packet_id> m_config_cb;
        map_type<client::play_packet_id> m_play_cb;
    };
}

#endif //PACKET_DISPATCHER_HPP
