#ifndef CLIENT_PACKETS_H
#define CLIENT_PACKETS_H
#include "Packet.h"
#include "utils.h"

#include <bits/stdint-uintn.h>
#include <spdlog/fmt/fmt.h>
#include <string>

namespace mc
{
    namespace client
    {
        enum class IdlePacketID : int
        {
            UNKNOWN   = -1,
            HANDSHAKE = 0
        };

        enum class StatusPacketID : int
        {
            UNKNOWN = -1,
            STATUS  = 0,
            PING    = 1
        };

        enum class LoginPacketID : int
        {
            UNKNOWN = -1,
            START   = 0
        };

        enum class PlayPacketID : int
        {
            UNKNOWN           = -1,
            LoginAcknowledged = 0x03
        };

        // ***************
        // * IdlePackets *
        // ***************

        class HandshakePacket : public Packet
        {
        public:
            template<util::IteratorU8 Iter>
            HandshakePacket(Iter& data)
                : Packet(IdlePacketID::HANDSHAKE),
                  m_protocolVersion(util::readVarInt(data)),
                  m_serverAddress(util::readString(data)),
                  m_port((*data << 8) + *(++data)),
                  m_nextState(util::readVarInt(++data))
            {
            }

            int GetProtocolVersion() const;
            const std::string& GetAdress() const;
            uint16_t GetPort() const;
            int GetNextState() const;

            std::string AsString() const override;
            constexpr std::string PacketName() const override;

        private:
            int m_protocolVersion;
            std::string m_serverAddress;
            uint16_t m_port;
            int m_nextState;
        };

        // *****************
        // * StatusPackets *
        // *****************

        class StatusRequestPacket : public Packet
        {
        public:
            StatusRequestPacket() : Packet(StatusPacketID::STATUS) {}
            ~StatusRequestPacket() = default;

            std::string AsString() const override;
            constexpr std::string PacketName() const override;
        };

        class PingRequest : public Packet
        {
        public:
            template<util::IteratorU8 Iter>
            PingRequest(Iter& data)
                : Packet(StatusPacketID::PING),
                  m_payload((*data << 8) + (*++data << 8) + (*++data << 8) + (*++data << 8) +
                            (*++data << 8) + (*++data << 8) + (*++data << 8) + (*++data << 8))
            {
            }

            uint64_t GetPayload() const;

            std::string AsString() const override;
            constexpr std::string PacketName() const override;

        private:
            uint64_t m_payload;
        };

        // ****************
        // * LoginPackets *
        // ****************

        class LoginStartPacket : public Packet
        {
        public:
            template<util::IteratorU8 Iter>
            LoginStartPacket(Iter& data)
                : Packet(LoginPacketID::START),
                  m_playerName(util::readString(data)),
                  m_hasUUID(*data++),
                  m_uuid(data)
            {
            }

            const std::string& GetPlayerName() const;
            util::uuid GetUUID() const;

            std::string AsString() const override;
            constexpr std::string PacketName() const override;

        private:
            std::string m_playerName;
            bool m_hasUUID;
            util::uuid m_uuid;
        };

        // ****************
        // * PlayPackets *
        // ****************

        class LoginAckPacket : public Packet
        {
        public:
            LoginAckPacket() : Packet(PlayPacketID::LoginAcknowledged) {}
        };

        // INLINES
        inline constexpr std::string HandshakePacket::PacketName() const
        {
            return "HandshakePacket";
        }

        inline std::string HandshakePacket::AsString() const
        {
            return std::format("{{protocol: {},serverAddress: {},port : {},nextState: {}}}",
                m_protocolVersion,
                m_serverAddress,
                m_port,
                m_nextState);
        }

        inline int HandshakePacket::GetProtocolVersion() const { return m_protocolVersion; }

        inline const std::string& HandshakePacket::GetAdress() const { return m_serverAddress; }

        inline uint16_t HandshakePacket::GetPort() const { return m_port; }

        inline int HandshakePacket::GetNextState() const { return m_nextState; }

        inline constexpr std::string StatusRequestPacket::PacketName() const
        {
            return "StatusRequestPacket";
        }

        inline std::string PingRequest::AsString() const
        {
            return std::format("{{ payload:{} }}", m_payload);
        }

        inline constexpr std::string PingRequest::PacketName() const { return "PingRequest"; }

        inline std::string StatusRequestPacket::AsString() const { return ""; }

        inline uint64_t PingRequest::GetPayload() const { return m_payload; }

        inline std::string LoginStartPacket::AsString() const
        {
            return std::format("{{playerName: {}, hasUUID: {}, uuid:{} }}",
                m_playerName,
                m_hasUUID,
                m_uuid);
        }

        inline constexpr std::string LoginStartPacket::PacketName() const
        {
            return "LoginStartPacket";
        }

        inline const std::string& LoginStartPacket::GetPlayerName() const { return m_playerName; }

        inline util::uuid LoginStartPacket::GetUUID() const { return m_uuid; }
    } // namespace client
} // namespace mc

#endif // CLIENT_PACKETS_H