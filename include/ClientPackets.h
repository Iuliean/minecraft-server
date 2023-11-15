#ifndef CLIENT_PACKETS_H
#define CLIENT_PACKETS_H
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstring>
#include <spdlog/fmt/fmt.h>
#include <string>
#include <string_view>

#include "Packet.h"

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

        class HandshakePacket: public Packet
        {
        public:
            HandshakePacket(auto&& data)
                : Packet(IdlePacketID::HANDSHAKE),
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
                : Packet(StatusPacketID::STATUS)
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
                :Packet(StatusPacketID::PING),
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
                : Packet(LoginPacketID::START),
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


#endif //CLIENT_PACKETS_H