#ifndef SERVER_PACKETS_H
#define SERVER_PACKETS_H

#include "Packet.h"

#include "ClientPackets.h"
#include "Serializer.h"
#include "nlohmann/json.hpp"
#include "utils.h"

namespace mc
{
    namespace server
    {
        enum class IdlePacketID: int
        {
            UNKNOWN     = -1
        };

        enum class StatusPacketID: int
        {
            UNKNOWN = -1,
            STATUS = 0
        };

        enum class LoginPacketID: int
        {
            UNKNOWN = -1,
            SUCCESS  = 2
        };

        class LoginSuccessPacket : public Packet
        {
        public:
            LoginSuccessPacket(const client::LoginStartPacket& packet);

            inline util::uuid GetUUID()const
            {return m_uuid;}
            inline const std::string& GetName()const
            {return m_name;}


            inline std::string AsString()const override
            {return fmt::format("{{ name: {}, uuid: {}}}", m_name, m_uuid);} 

            inline constexpr const char* PacketName()const override
            {return "LoginSuccessPacket";}

        private:
            util::uuid m_uuid;
            std::string m_name;
        };

        class StatusPacket : public Packet
        {
        public:
            StatusPacket();
            ~StatusPacket() = default;

            inline nlohmann::json& Json()
            {return m_payload;}

            inline const nlohmann::json& Json()const
            {
                return m_payload;
            }

            inline std::string AsString()const override
            {return fmt::format("{}", m_payload);} 

            inline constexpr const char* PacketName()const override
            {return "StatusPacket";}
        private:
            nlohmann::json m_payload;
        };
    }
}

template<>
struct iu::Serializer<mc::server::LoginSuccessPacket>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::LoginSuccessPacket& toSerialize)
    {   
        using namespace mc::util;
        //size of uuid 16 bytes(128bits) + size of name string + count of elements (atm 0) + id
        const int packetSize = 128/8 + sizeOfString(toSerialize.GetName()) + 2;
        iu::Serializer<uuid> uuidSerializer;

        writeVarInt(buffer, packetSize);
        writeVarInt(buffer, toSerialize.GetId<int>());
        uuidSerializer.Serialize(buffer, toSerialize.GetUUID());
        writeStringToBuff(buffer, toSerialize.GetName());
        writeVarInt(buffer, 0);
    }
};

template<>
struct iu::Serializer<mc::server::StatusPacket>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::StatusPacket& toSerialize)
    {
        using namespace mc::util;
        const int packetSize = 1 + sizeOfString(toSerialize.Json().dump());
        writeVarInt(buffer, packetSize);
        writeVarInt(buffer, toSerialize.GetId<int>());
        writeStringToBuff(buffer, toSerialize.Json().dump()); 
    }
};  

#endif //SERVER_PACKETS_H