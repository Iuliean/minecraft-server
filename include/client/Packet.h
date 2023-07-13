#ifndef PACKET_H
#define PACKET_H
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstring>
#include <spdlog/fmt/fmt.h>
#include <string>
#include <string_view>
#include <spdlog/fmt/bundled/format.h>
#include <boost/uuid/uuid.hpp>


#include "utils.h"

namespace mc
{
    namespace client
    {
        enum class IdlePacketID: int
        {
            UNKNOWN     = -1,
            HANDSHAKE   = 0
        };

        enum class StatusPacketID: int
        {
            UNKNOWN = -1,
            STATUS  = 0,
            PING    = 1
        };

        enum class LoginPacketID: int
        {
            UNKNOWN = -1,
            START   = 0
        };

        class Packet
        {
        public:
            using PacketPtr = std::unique_ptr<Packet>;

            Packet(int id);
            virtual ~Packet() = default;

            int GetId()const;
            
            virtual std::string AsString()const; 
            virtual constexpr const char* PacketName()const;
        private:
            int m_id;
        };

        class HandshakePacket: public Packet
        {
        public:
            HandshakePacket(auto&& data)
                : Packet((int)IdlePacketID::HANDSHAKE),
                m_protocolVersion(util::readVarInt(data)),
                m_serverAddress(util::readString(data)),
                m_port((*data << 8) + *(++data)),
                m_nextState(util::readVarInt(++data))
            {
                static_assert(util::IteratorU8<std::remove_reference_t<decltype(data)>>);
            }

            int GetProtocolVersion()const;
            const std::string& GetAdress()const;
            uint16_t GetPort()const;
            int GetNextState()const;

            std::string AsString()const override; 
            constexpr const char* PacketName()const override;

        private:
            int m_protocolVersion;
            std::string m_serverAddress;
            uint16_t m_port;
            int m_nextState;
        };

        class StatusRequestPacket: public Packet
        {
        public:
            StatusRequestPacket()
                : Packet((int)StatusPacketID::STATUS)
            {
            }
            ~StatusRequestPacket() = default;
            
            std::string AsString()const override; 
            constexpr const char* PacketName()const override;
        };

        class PingRequest: public Packet
        {
        public:
            PingRequest(auto&& data)
                :Packet((int)StatusPacketID::PING),
                m_payload((*data << 8) + (*++data << 8) + (*++data << 8) + (*++data << 8) + (*++data << 8) + (*++data << 8) + (*++data << 8) + (*++data << 8))
                {
                }
    
            uint64_t GetPayload()const;

            std::string AsString()const override; 
            constexpr const char* PacketName()const override;
        private:
            uint64_t m_payload;
        };

        class LoginStartPacket: public Packet
        {
        public:
            LoginStartPacket(auto&& data)
                : Packet((int)LoginPacketID::START),
                m_playerName(util::readString(data)),
                m_hasUUID(*data++),
                m_uuid(data)
            {
                static_assert(util::IteratorU8<std::remove_reference_t<decltype(data)>>); 
            }

            const std::string& GetPlayerName() const;
            util::uuid GetUUID()const;

            std::string AsString()const override; 
            constexpr const char* PacketName()const override;
        private:
            std::string m_playerName;
            bool m_hasUUID;
            util::uuid m_uuid;
        };

        // INLINES
        inline int Packet::GetId()const
        {
            return m_id;
        }
        
        inline std::string Packet::AsString()const
        {
            return "{Id: " + std::to_string(m_id) + "}";
        } 

        inline constexpr const char* Packet::PacketName()const
        {
            return "Packet";
        }

        inline constexpr const char* HandshakePacket::PacketName()const
        {
            return "HandshakePacket";
        }

        inline std::string HandshakePacket::AsString()const
        {
            return fmt::format("{{protocol: {},serverAddress: {},port : {},nextState: {}}}", 
                    m_protocolVersion, m_serverAddress, m_port, m_nextState);
        }

        inline int HandshakePacket::GetProtocolVersion()const
        {
            return m_protocolVersion;
        }

        inline const std::string& HandshakePacket::GetAdress()const
        {
            return m_serverAddress;
        }

        inline uint16_t HandshakePacket::GetPort()const
        {
            return m_port;
        }

        inline int HandshakePacket::GetNextState()const
        {
            return m_nextState;
        }

        inline constexpr const char* StatusRequestPacket::PacketName()const
        {
            return "StatusRequestPacket";
        }

        inline std::string PingRequest::AsString()const
        {
            return fmt::format("{{ payload:{} }}", m_payload);
        }

        inline constexpr const char* PingRequest::PacketName()const
        {
            return "PingRequest";
        }

        inline std::string StatusRequestPacket::AsString()const
        {
            return "";
        }

        inline uint64_t PingRequest::GetPayload()const
        {
            return m_payload;
        }

        inline std::string LoginStartPacket::AsString()const
        {
            return fmt::format("{{playerName: {}, hasUUID: {}, uuid:{} }}", m_playerName, m_hasUUID, m_uuid);
        }
        inline constexpr const char* LoginStartPacket::PacketName()const
        {
            return "LoginStartPacket";
        }

        inline const std::string& LoginStartPacket::GetPlayerName()const
        {
            return m_playerName;
        }

        inline util::uuid LoginStartPacket::GetUUID()const
        {
            return m_uuid;
        }

    }
}

//FMT FORMATTERS

template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<mc::client::Packet, T>::value, char>> :
    fmt::formatter<std::string> {
  auto format(const mc::client::Packet& a, format_context& ctx) const {
    return fmt::formatter<std::string>::format(fmt::format("{}:{}", a.PacketName(), a.AsString()), ctx);
  }
};
#endif //PACKET_H