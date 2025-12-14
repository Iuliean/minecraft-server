#ifndef CLIENT_PACKETS_HPP
#define CLIENT_PACKETS_HPP
#include "packet.hpp"
#include "utils.hpp"
#include "data_types/uuid.hpp"

#include <cstdint>
#include <string>

namespace mc
{
    namespace client
    {
        enum class idle_packet_id : int
        {
            unknown   = -1,
            handshake = 0
        };

        enum class status_packet_id : int
        {
            unknown = -1,
            status  = 0,
            ping    = 1
        };

        enum class login_packet_id : int
        {
            unknown = -1,
            start   = 0,
            login_ack = 0x03

        };

        enum class config_packet_id : int
        {
            unknown = -1,
            ack_config_end = 0x03,
            known_packs = 0x07
        };

        enum class play_packet_id : int
        {
            unknown           = -1,
        };

        // ***************
        // * IdlePackets *
        // ***************

        class handshake_packet : public packet
        {
        public:
            template<util::IteratorU8 Iter>
            handshake_packet(Iter& data)
                : packet(idle_packet_id::handshake),
                  m_protocol_version(util::readVarInt(data)),
                  m_server_address(util::readString(data)),
                  m_port([&data](){
                      uint16_t result = *data << 8;
                      result += *(++data);
                      return result;
                  }()),
                  m_next_state(util::readVarInt(++data))
            {
            }
            virtual ~handshake_packet() = default;

            int get_protocol_version() const noexcept { return m_protocol_version; }
            const std::string& get_address() const noexcept { return m_server_address; }
            uint16_t get_port() const noexcept { return m_port; }
            int get_next_state() const noexcept { return m_next_state; }

            constexpr std::string as_string() const override
            {
                return std::format("protocol:{}, address:{}, port:{}, next_state:{}",
                    m_protocol_version, m_server_address, m_port, m_next_state);
            }
            constexpr std::string packet_name() const override { return "handshake"; }

        private:
            int m_protocol_version;
            std::string m_server_address;
            uint16_t m_port;
            int m_next_state;
        };

        template<util::IteratorU8 Iter>
        packet_ptr parse_idle_packet(Iter& iter)
        {
            util::readVarInt(iter);
            const auto id = static_cast<idle_packet_id>(util::readVarInt(iter));

            switch(id)
            {
                case idle_packet_id::handshake:
                    return std::make_unique<handshake_packet>(iter);
                default:
                    return nullptr;
            }
        }
        // *****************
        // * StatusPackets *
        // *****************

        class status_request_packet : public packet
        {
        public:
            status_request_packet() : packet(status_packet_id::status) {}
            virtual ~status_request_packet() = default;

            constexpr std::string as_string() const override { return ""; }
            constexpr std::string packet_name() const override { return "status_request"; }
        };

        class ping_request : public packet
        {
        public:
            template<util::IteratorU8 Iter>
            ping_request(Iter& data)
                : packet(status_packet_id::ping),
                  m_payload([&data](){
                      uint64_t result = 0;
                      result += *(data++) << 8;
                      result += *(data++) << 8;
                      result += *(data++) << 8;
                      result += *(data++) << 8;
                      result += *(data++) << 8;
                      result += *(data++) << 8;
                      result += *(data++) << 8;
                      result += *(data++) << 8;
                      return result;
                  }())
            {
            }
            virtual ~ping_request() = default;
            uint64_t get_payload() const noexcept {return m_payload; }

            constexpr std::string as_string() const override { return std::format("payload:{}", m_payload); }
            constexpr std::string packet_name() const override { return "ping_request"; }

        private:
            uint64_t m_payload;
        };

        template<util::IteratorU8 Iter>
        packet_ptr parse_status_packet(Iter& iter)
        {
            util::readVarInt(iter);
            const auto id = static_cast<status_packet_id>(util::readVarInt(iter));

            switch(id)
            {
                case status_packet_id::status:
                    return std::make_unique<status_request_packet>();
                case status_packet_id::ping:
                    return std::make_unique<ping_request>(iter);
                default:
                    return nullptr;
            }
        }
        // *****************
        // * ConfigPackets *
        // *****************

        class known_packs_packet : public packet
        {
        public:
            template<util::IteratorU8 Iter>
            known_packs_packet(Iter& data)
                : packet(config_packet_id::known_packs),
                  m_namespace([&data](){util::readVarInt(data); return util::readString(data);}()),
                  m_id(util::readString(data)),
                  m_version(util::readString(data))
            {
            }
            virtual ~known_packs_packet() = default;

            constexpr std::string as_string() const override
            { return std::format("namespace: {}, id: {}, version: {}", m_namespace, m_id, m_version); }

            constexpr std::string packet_name()const override { return "known_packs"; }
        private:
            std::string m_namespace;
            std::string m_id;
            std::string m_version;
        };

        class ack_config : public packet
        {
        public:
            ack_config()
                : packet(config_packet_id::ack_config_end)
            {}
            virtual ~ack_config() = default;

            constexpr std::string as_string()const override{ return ""; }
            constexpr std::string packet_name()const override { return "ack_config"; }
        };

        template<util::IteratorU8 Iter>
        packet_ptr parse_config_packet(Iter& iter)
        {
            util::readVarInt(iter);
            const auto id = static_cast<config_packet_id>(util::readVarInt(iter));

            switch(id)
            {
                case config_packet_id::known_packs:
                    return std::make_unique<known_packs_packet>(iter);
                case config_packet_id::ack_config_end:
                    return std::make_unique<ack_config>();
                default:
                    return nullptr;
            }
        }

        // ****************
        // * LoginPackets *
        // ****************

        class login_start_packet : public packet
        {
        public:
            template<util::IteratorU8 Iter>
            login_start_packet(Iter& data)
                : packet(login_packet_id::start),
                  m_player_name(util::readString(data)),
                  m_has_uuid(*data++),
                  m_uuid(data)
            {
            }
            virtual ~login_start_packet() = default;

            const std::string& get_player_name() const { return m_player_name; }
            constexpr uuid get_uuid() const noexcept {return m_uuid; }

            constexpr std::string as_string() const override
            {
                return std::format
                (
                    "player_name:{}, has_uuid:{}, uuid:{}",
                    m_player_name,
                    m_has_uuid,
                    m_has_uuid ? m_uuid : uuid()
                );
            }
            constexpr std::string packet_name() const override { return "login_start"; }

        private:
            std::string m_player_name;
            bool m_has_uuid;
            uuid m_uuid;
        };

        class login_ack : public packet
        {
        public:
            login_ack() : packet(login_packet_id::login_ack) {}
            virtual ~login_ack() = default;

            constexpr std::string as_string() const override { return ""; }
            constexpr std::string packet_name() const override { return "login_ack"; }
        };

        template<util::IteratorU8 Iter>
        std::unique_ptr<packet> parse_login_packet(Iter& iter)
        {
            util::readVarInt(iter);
            const auto id = static_cast<login_packet_id>(util::readVarInt(iter));

            switch(id)
            {
                case login_packet_id::start:
                    return std::make_unique<login_start_packet>(iter);
                case login_packet_id::login_ack:
                    return std::make_unique<ack_config>();
                default:
                    return nullptr;
            }
        }

        // ****************
        // * PlayPackets *
        // ****************

        template<util::IteratorU8 Iter>
        packet_ptr parse_play_packet(Iter& iter)
        {
            util::readVarInt(iter);
            const auto id = static_cast<play_packet_id>(util::readVarInt(iter));

            switch(id)
            {
                default:
                    return nullptr;
            }
        }

    } // namespace client
} // namespace mc

#endif // CLIENT_PACKETS_H
