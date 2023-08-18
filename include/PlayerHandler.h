#ifndef PLAYER_HANDLER_H
#define PLAYER_HANDLER_H
#include <memory>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <Connection.h>

#include "Logger.h"
#include "Packet.h"
#include "ClientPackets.h"
#include "ServerPackets.h"

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
        
        void OnIdle(Packet::PacketPtr&& genericPacket);
        void OnStatus(Packet::PacketPtr&& genericPacket);
        void OnLogin(Packet::PacketPtr&& genericPacket);
        void OnPlay(Packet::PacketPtr&& genericPacket);

    private:
        Packet::PacketPtr NextPacketIdle(auto&& dataIter)
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
                    m_logger.warn("Invalid idle packetID: {}", packetID);
                    return nullptr;
            }
        }    

        Packet::PacketPtr NextPacketStatus(auto&& dataIter)
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
                    m_logger.warn("Invalid status packetID: {}", packetID);
                    return nullptr; 
            }
        }

        Packet::PacketPtr NextPacketLogin(auto&& dataIter)
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
                    m_logger.warn("Invalid status packetID: {}", packetID);
                    return nullptr; 
            }
        }

        Packet::PacketPtr NextPacketPlay(auto&& dataIter)
        {
            static_assert(util::IteratorU8<std::remove_reference_t<decltype(dataIter)>>);
            return nullptr;
        }
        
        iu::Connection& m_client;
        PlayerHandlerState m_state;
        iu::Logger m_logger;
        server::StatusPacket m_statusMessage;
    };
}
#endif //PLAYER_HANDLER_H