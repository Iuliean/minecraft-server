#ifndef PLAYER_HANDLER_H
#define PLAYER_HANDLER_H
#include <memory>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "Connection.h"
#include "client/Packet.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "utils.h"

namespace mc
{
    enum class PlayerHandlerState: int
    {
        IDLE    = 0,
        STATUS  = 1,
        LOGIN   = 2,
        PLAY    = 3
    };

    class PlayerHandler
    {
    public:
        PlayerHandler(iu::Connection& client);
        ~PlayerHandler() = default;

        void Execute(const std::vector<uint8_t>& data);
        
        void OnIdle(client::Packet::PacketPtr&& genericPacket);
        void OnStatus(client::Packet::PacketPtr&& genericPacket);
        void OnLogin(client::Packet::PacketPtr&& genericPacket);
        void OnPlay(client::Packet::PacketPtr&& genericPacket);

    private:
        mc::client::Packet::PacketPtr NextPacketIdle(auto&& dataIter)
        {
            static_assert(util::IteratorU8<std::remove_reference_t<decltype(dataIter)>>);
            using namespace mc::client;
            int packetSize = util::readVarInt(dataIter);
            int packetID;
            
            if (packetSize == 0)
                return nullptr;
            packetID = util::readVarInt(dataIter);

            switch ((IdlePacketID)packetID) {
                case IdlePacketID::HANDSHAKE:
                    return std::make_unique<HandshakePacket>(dataIter);
                default:
                    //maybe needs to throw
                    m_logger->warn("Invalid idle packetID: {}", packetID);
                    return nullptr;
            }
        }    

        mc::client::Packet::PacketPtr NextPacketStatus(auto&& dataIter)
        {
            static_assert(util::IteratorU8<std::remove_reference_t<decltype(dataIter)>>);
            using namespace mc::client;
            int packetSize = util::readVarInt(dataIter);
            int packetID;
            
            if(packetSize == 0)
                return nullptr;

            packetID = util::readVarInt(dataIter);

            switch ((StatusPacketID)packetID) {
                case StatusPacketID::STATUS:
                    return std::make_unique<StatusRequestPacket>();
                case StatusPacketID::PING:
                    return std::make_unique<PingRequest>(dataIter);
                default:
                    //maybe needs to throw
                    m_logger->warn("Invalid status packetID: {}", packetID);
                    return nullptr; 
            }
        }

        mc::client::Packet::PacketPtr NextPacketLogin(auto&& dataIter)
        {
            static_assert(util::IteratorU8<std::remove_reference_t<decltype(dataIter)>>);
            using namespace mc::client;
            int packetSize = util::readVarInt(dataIter);
            int packetID;

            if(packetSize == 0)
                return nullptr;

            packetID = util::readVarInt(dataIter);

            switch((LoginPacketID)packetID)
            {
                case LoginPacketID::START:
                    return std::make_unique<LoginStartPacket>(dataIter);
                default:
                    //maybe needs to throw
                    m_logger->warn("Invalid status packetID: {}", packetID);
                    return nullptr; 
            }
        }

        mc::client::Packet::PacketPtr NextPacketPlay(auto&& dataIter)
        {
            static_assert(util::IteratorU8<std::remove_reference_t<decltype(dataIter)>>);
        }
        
        iu::Connection& m_client;
        PlayerHandlerState m_state;
        std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> m_sink;
        std::shared_ptr<spdlog::logger> m_logger;
        nlohmann::json m_statusMessage;
    };
}
#endif //PLAYER_HANDLER_H