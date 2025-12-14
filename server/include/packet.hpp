#ifndef PACKET_HPP
#define PACKET_HPP

#include <concepts>
#include <spdlog/fmt/fmt.h>
#include <memory>
#include <type_traits>
#include <utility>
#include "utils.hpp"


namespace mc
{
    namespace server
    {
        enum class idle_packet_id;
        enum class status_packet_id;
        enum class config_packet_id;
        enum class login_packet_id;
        enum class play_packet_id;
    } // namespace server

    namespace client
    {
        enum class idle_packet_id;
        enum class status_packet_id;
        enum class login_packet_id;
        enum class config_packet_id;
        enum class play_packet_id;
    } // namespace client

    template<typename T>
    concept PacketID =
        std::same_as<T, int> || std::same_as<T, client::idle_packet_id> ||
        std::same_as<T, server::idle_packet_id> || std::same_as<T, client::status_packet_id>   ||
        std::same_as<T, server::status_packet_id> || std::same_as<T, client::login_packet_id>  ||
        std::same_as<T, server::config_packet_id> || std::same_as<T, client::config_packet_id> ||
        std::same_as<T, server::login_packet_id> || std::same_as<T, client::play_packet_id> ||
        std::same_as<T, server::play_packet_id>;

    class packet
    {
    public:

        template<PacketID T>
        packet(T id)
            : m_id(std::to_underlying(id)) {}
        packet(int id)
            : m_id(id) {}

        virtual ~packet() = default;

        template<PacketID T>
        typename std::remove_const<T>::type get_id()const
        { return static_cast<std::remove_const_t<T>>(m_id);}

        virtual std::string as_string()const = 0;
        virtual constexpr std::string packet_name()const = 0;
        virtual constexpr size_t size()const{ return 1;};

    protected:
        int m_id;
    };

    using packet_ptr = std::unique_ptr<packet>;

    template<typename T>
    concept is_packet = std::derived_from<T, packet>;
}

//FMT FORMATTERS

template <typename T>
    requires std::derived_from<T, mc::packet>
struct std::formatter<T> : public std::formatter<std::string>
{
  auto format(const mc::packet& packet, format_context& ctx) const {
    return std::formatter<std::string>::format(std::format("{}:{{{}}}", packet.packet_name(), packet.as_string()), ctx);
  }
};

#endif //PACKET_H
