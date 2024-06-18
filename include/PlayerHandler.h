#ifndef PLAYER_HANDLER_H
#define PLAYER_HANDLER_H
#include <memory>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <SFW/Connection.h>
#include <SFW/Logger.h>

#include "Packet.h"
#include "ClientPackets.h"
#include "ServerPackets.h"
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

        void OnIdle(Packet::PacketPtr&& genericPacket);
        void OnStatus(Packet::PacketPtr&& genericPacket);
        void OnLogin(Packet::PacketPtr&& genericPacket);
        void OnPlay(Packet::PacketPtr&& genericPacket);

        void PlayLoop();

    private:
        template<util::IteratorU8 Iter>
        Packet::PacketPtr NextPacketIdle(Iter& dataIter)
        {
            using namespace mc::client;
            int packetSize = util::readVarInt(dataIter);
            int packetID;
            
            if (packetSize == 0)
                return nullptr;
            packetID = util::readVarInt(dataIter);

            switch (static_cast<IdlePacketID>(packetID)) {
                case IdlePacketID::HANDSHAKE:
                    return std::make_unique<HandshakePacket>(dataIter);
                default:
                    //maybe needs to throw
                    m_logger.warn("Invalid idle packetID: {:0x}", packetID);
                    return nullptr;
            }
        }    

        template<util::IteratorU8 Iter>
        Packet::PacketPtr NextPacketStatus(Iter& dataIter)
        {
            using namespace mc::client;
            int packetSize = util::readVarInt(dataIter);
            int packetID;
            
            if(packetSize == 0)
                return nullptr;

            packetID = util::readVarInt(dataIter);

            switch (static_cast<StatusPacketID>(packetID)) {
                case StatusPacketID::STATUS:
                    return std::make_unique<StatusRequestPacket>();
                case StatusPacketID::PING:
                    return std::make_unique<PingRequest>(dataIter);
                default:
                    //maybe needs to throw
                    m_logger.warn("Invalid status packetID: {:0x}", packetID);
                    return nullptr; 
            }
        }

        template<util::IteratorU8 Iter>
        Packet::PacketPtr NextPacketLogin(Iter& dataIter)
        {
            using namespace mc::client;
            int packetSize = util::readVarInt(dataIter);
            int packetID;

            if(packetSize == 0)
                return nullptr;

            packetID = util::readVarInt(dataIter);

            switch(static_cast<LoginPacketID>(packetID))
            {
                case LoginPacketID::START:
                    return std::make_unique<LoginStartPacket>(dataIter);
                default:
                    //maybe needs to throw
                    m_logger.warn("Invalid status packetID: {:0x}", packetID);
                    return nullptr; 
            }
        }

        template<util::IteratorU8 Iter>
        Packet::PacketPtr NextPacketPlay(Iter& dataIter)
        {
            using namespace mc::client;
            int packetSize = util::readVarInt(dataIter);
            int packetID;

            if(packetSize == 0)
                return nullptr;

            packetID = util::readVarInt(dataIter);
            
            switch(static_cast<PlayPacketID>(packetID))
            {
                case PlayPacketID::LoginAcknowledged:
                    return std::make_unique<LoginAckPacket>();
                    break;
                default:
                    m_logger.warn("Invalid play packetID: {:0x}", packetID);
                    return nullptr;
            }
        }
        
        iu::Connection& m_client;
        PlayerHandlerState m_state;
        iu::Logger m_logger;
        server::StatusPacket m_statusMessage;
    };
}
#endif //PLAYER_HANDLER_H