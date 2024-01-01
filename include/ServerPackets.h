#ifndef SERVER_PACKETS_H
#define SERVER_PACKETS_H

#include "ClientPackets.h"
#include "DataTypes/Identifier.h"
#include "Packet.h"
#include "Position.h"
#include "Serializer.h"
#include "nlohmann/json.hpp"
#include "utils.h"

#include <cstdint>
#include <string>
namespace mc::server
{

    enum class IdlePacketID : int
    {
        UNKNOWN = -1
    };

    enum class StatusPacketID : int
    {
        UNKNOWN = -1,
        STATUS  = 0
    };

    enum class LoginPacketID : int
    {
        UNKNOWN = -1,
        SUCCESS = 2
    };

    enum class PlayPacketID : int
    {
        UNKNOWN   = -1,
        LoginPlay = 0x29
    };

    // ****************
    // * LoginPackets *
    // ****************

    class LoginSuccessPacket : public Packet
    {
    public:
        LoginSuccessPacket(const client::LoginStartPacket& packet);

        inline util::uuid GetUUID() const { return m_uuid; }
        inline const std::string& GetName() const { return m_name; }

        inline std::string AsString() const override
        {
            return fmt::format("{{ name: {}, uuid: {}}}", m_name, m_uuid);
        }

        inline constexpr std::string PacketName() const override { return "LoginSuccessPacket"; }

    private:
        util::uuid m_uuid;
        std::string m_name;
        int m_numOfElements;
    };

    // *****************
    // * StatusPackets *
    // *****************

    class StatusPacket : public Packet
    {
    public:
        StatusPacket();
        ~StatusPacket() = default;

        inline nlohmann::json& Json() { return m_payload; }

        inline const nlohmann::json& Json() const { return m_payload; }

        inline std::string AsString() const override { return fmt::format("{}", m_payload); }

        inline constexpr std::string PacketName() const override { return "StatusPacket"; }

    private:
        nlohmann::json m_payload;
    };

    // ****************
    // * PlayPackets *
    // ****************

    class LoginPlayPacket : public Packet
    {
    public:
        LoginPlayPacket();

        inline std::string AsString() const override
        {
            constexpr const char* const text = "EntityID:{}, "
                                               "isHardcore:{}, "
                                               "maxPlayers:{}, "
                                               "viewDistance:{}, "
                                               "simulationDistance:{}, "
                                               "reducedDebugInfo:{}, "
                                               "enableRespawnScreen:{}, "
                                               "limitedCrafting:{}, "
                                               "seedHash:{}, "
                                               "gameMode:{}, "
                                               "previousGameMode:{}, "
                                               "isDebug:{}, "
                                               "isFlat:{}, "
                                               "hasDeathLocation:{}, "
                                               "portalCooldown:{}";
            return fmt::format(text,
                m_entityID,
                m_isHardcore,
                m_maxPlayers,
                m_viewDistance,
                m_simulationDistance,
                m_reducedDebugInfo,
                m_enableRespawnScreen,
                m_limitedCrafting,
                m_seedHash,
                m_gameMode,
                m_previousGameMode,
                m_isDebug,
                m_isFlat,
                m_hasDeathLocation,
                m_portalCooldown);
        }

        inline constexpr std::string PacketName() const override { return "LoginPlay"; }

    private:
        friend iu::Serializer<mc::server::LoginPlayPacket>;

        int32_t m_entityID;
        bool m_isHardcore;
        std::vector<mc::Identifier> m_dimensionIdentifiers;
        int32_t m_maxPlayers;
        int32_t m_viewDistance;
        int32_t m_simulationDistance;
        bool m_reducedDebugInfo;
        bool m_enableRespawnScreen;
        bool m_limitedCrafting;
        Identifier m_dimensionType;
        Identifier m_dimensionName;
        int64_t m_seedHash;
        uint8_t m_gameMode;
        int8_t m_previousGameMode;
        bool m_isDebug;
        bool m_isFlat;
        bool m_hasDeathLocation;
        std::optional<Identifier> m_deathDimension;
        std::optional<Position> m_deathPosition;
        int32_t m_portalCooldown;
    };

} // namespace mc::server

template<>
struct iu::Serializer<mc::server::LoginSuccessPacket>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::LoginSuccessPacket& toSerialize)
    {
        using namespace mc::util;
        // size of uuid 16 bytes(128bits) + size of name string + count of
        // elements (atm 0) + id
        const int packetSize = 128 / 8 + sizeOfString(toSerialize.GetName()) + 2;
        Serializer<uuid> uuidSerializer;

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

template<>
struct iu::Serializer<mc::server::LoginPlayPacket>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::LoginPlayPacket& toSerialize)
    {
        using namespace mc::util;

        IntSerializer intSerializer;
        LongSerializer int64Serializer;
        ByteSerializer byteSerialier;
        BoolSerializer boolSerializer;
        Serializer<mc::Identifier> identifierSerializer;
        Serializer<mc::Position> positionSerializer;
        Serializer<std::vector<mc::Identifier>> identifierVecSerializer;

        intSerializer.Serialize(buffer, toSerialize.m_entityID);
        boolSerializer.Serialize(buffer, toSerialize.m_isHardcore);
        writeVarInt(buffer, toSerialize.m_dimensionIdentifiers.size());
        identifierVecSerializer.Serialize(buffer, toSerialize.m_dimensionIdentifiers);
        writeVarInt(buffer, toSerialize.m_maxPlayers);
        writeVarInt(buffer, toSerialize.m_viewDistance);
        writeVarInt(buffer, toSerialize.m_simulationDistance);
        boolSerializer.Serialize(buffer, toSerialize.m_reducedDebugInfo);
        boolSerializer.Serialize(buffer, toSerialize.m_enableRespawnScreen);
        boolSerializer.Serialize(buffer, toSerialize.m_limitedCrafting);
        identifierSerializer.Serialize(buffer, toSerialize.m_dimensionType);
        identifierSerializer.Serialize(buffer, toSerialize.m_dimensionName);
        int64Serializer.Serialize(buffer, toSerialize.m_seedHash);
        byteSerialier.Serialize(buffer, toSerialize.m_gameMode);
        boolSerializer.Serialize(buffer, toSerialize.m_isDebug);
        boolSerializer.Serialize(buffer, toSerialize.m_isFlat);

        if(toSerialize.m_deathDimension.has_value())
            identifierSerializer.Serialize(buffer, toSerialize.m_deathDimension.value().AsString());
        if(toSerialize.m_deathPosition.has_value())
            positionSerializer.Serialize(buffer, toSerialize.m_deathPosition.value());

        writeVarInt(buffer, toSerialize.m_portalCooldown);
    }
};

#endif // SERVER_PACKETS_H