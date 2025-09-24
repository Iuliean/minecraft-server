#ifndef SERVER_PACKETS_H
#define SERVER_PACKETS_H

#include "ClientPackets.h"
#include "DataTypes/Identifier.h"
#include "DataTypes/nbt.h"
#include "Packet.h"
#include "DataTypes/Position.h"
#include <nlohmann/json.hpp>
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

    enum class ConfigPacketID : int
    {
        UNKNOWN = -1,
        FinishConfiguration = 0x03,
    };

    enum class PlayPacketID : int
    {
        UNKNOWN   = -1,
        GameEvent = 0x22,
        LoginPlay = 0x2b,
        SynchronisePlayerPosition = 0x40
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
            return std::format("{{ name: {}, uuid: {}}}", m_name, m_uuid);
        }

        inline constexpr std::string PacketName() const override { return "LoginSuccessPacket"; }
        inline size_t Size() const override 
        {
            return 1 +
            sizeof(util::uuid) +
            util::sizeOfString(m_name) +
            util::sizeOfVarInt(m_numOfElements) +
            sizeof(bool);
        }
    private:
        util::uuid m_uuid;
        std::string m_name;
        util::varInt m_numOfElements;
        bool m_strictErrorHandling;
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

        inline std::string AsString() const override { return std::format("{}", m_payload); }

        inline constexpr std::string PacketName() const override { return "StatusPacket"; }

        inline size_t Size() const override { return 1 + util::sizeOfString(m_payload.dump());}

    private:
        nlohmann::json m_payload;
    };
    // *****************
    // * ConfigPackets *
    // *****************

   class FinishConfiguration : public Packet
    {
    public:
        FinishConfiguration();
        ~FinishConfiguration() = default;

        inline std::string AsString() const override { return std::format("FinishConfig");}
        inline constexpr std::string PacketName() const override { return "FinishConfiguration";}
        inline size_t Size() const override { return 1;}
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
            return std::format(text,
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
        inline size_t Size() const override
        {
            size_t idendtifiers_size = 0;
            for (const auto& identifier: m_dimensionIdentifiers)
                idendtifiers_size += util::sizeOfString(identifier.AsString());
            return 2 + //2 is because the field of the optionals is needed
            sizeof(int32_t) +
            (8 * sizeof(bool)) +
            util::sizeOfVarInt(m_dimensionIdentifiers.size()) +
            idendtifiers_size +
            util::sizeOfVarInt(m_maxPlayers) +
            util::sizeOfVarInt(m_viewDistance) +
            util::sizeOfVarInt(m_simulationDistance) +
            util::sizeOfVarInt(m_dimensionType) +
            util::sizeOfString(m_dimensionName.AsString()) +
            sizeof(int64_t) + sizeof(uint8_t) + sizeof(int8_t) +
            (m_deathDimension.has_value() ? util::sizeOfString(m_deathDimension.value().AsString()) : 0) +
            (m_deathPosition.has_value() ? sizeof(Position) : 0);
        }

    private:
        friend iu::Serializer<mc::server::LoginPlayPacket>;

        int32_t m_entityID;
        bool m_isHardcore;
        std::vector<mc::Identifier> m_dimensionIdentifiers;
        util::varInt m_maxPlayers;
        util::varInt m_viewDistance;
        util::varInt m_simulationDistance;
        bool m_reducedDebugInfo;
        bool m_enableRespawnScreen;
        bool m_limitedCrafting;
        util::varInt m_dimensionType;
        Identifier m_dimensionName;
        int64_t m_seedHash;
        uint8_t m_gameMode;
        int8_t m_previousGameMode;
        bool m_isDebug;
        bool m_isFlat;
        bool m_hasDeathLocation;
        std::optional<Identifier> m_deathDimension;
        std::optional<Position> m_deathPosition;
        util::varInt m_portalCooldown;
        bool m_enforceSecureChat;
    };

    class GameEvent : public Packet
    {
    public:
        enum class Event : std::uint8_t
        {
            NoRespawnBlockAvailable = 0,
            BeginRaining,
            EndRaining,
            ChangeGameMode,
            WinGame,
            DemoEvent,
            ArrowHitPlayer,
            RainLevelChange,
            ThunderLevelChange,
            PlayPufferFishStingSound,
            PlayElderGuardianMobAppeareance,
            EnableRespawnScreen,
            LimitedCrafting,
            StartWaitingForChunks
        };
        GameEvent(Event event, float value = 0);
        ~GameEvent() = default;

        inline std::string AsString() const override { return std::format("GameEvent{{ Event: {}, Value: {}}}", (int)m_event, m_value);}
        inline constexpr std::string PacketName() const override { return "GameEvent";}
        inline size_t Size() const override { return 1 + sizeof(Event) + sizeof(float);}
    private:
        friend iu::Serializer<mc::server::GameEvent>;
        Event m_event;
        float m_value; //depends on event
    };

    class SynchronisePlayerPosition : public Packet
    {
    public:
        SynchronisePlayerPosition(double x, double y, double z, float yaw, float pitch, std::uint8_t relativeMask = 0);
        ~SynchronisePlayerPosition() = default;

        inline std::string AsString() const override
        {
            constexpr auto fmt = "{}: "
                "X: {}, Y: {}, Z: {}, Yaw: {}, Pitch: {},"
                "RelativeMask:{:b}, TeleportID: {}";
            return std::format(fmt,
                PacketName(),
                m_x, m_y, m_z, m_yaw, m_pitch,
                m_relativeMask, m_teleportID
            );
        }

        inline constexpr std::string PacketName() const override { return "SynchronisePlayerPosition"; }
        inline size_t Size() const override { return 2 + (3 * sizeof(double)) + (2 * sizeof(float) + util::sizeOfVarInt(m_teleportID)); }
    private:
        friend iu::Serializer<mc::server::SynchronisePlayerPosition>;

        double m_x;
        double m_y;
        double m_z;
        float m_yaw;
        float m_pitch;
        std::uint8_t m_relativeMask;
        util::varInt m_teleportID;
    };

} // namespace mc::server

template<>
struct iu::Serializer<mc::server::LoginSuccessPacket>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::LoginSuccessPacket& toSerialize)
    {
        using namespace mc::util;
        Serializer<uuid> uuidSerializer;

        writeVarInt(buffer, toSerialize.Size());
        writeVarInt(buffer, toSerialize.GetId<int>());
        uuidSerializer.Serialize(buffer, toSerialize.GetUUID());
        writeStringToBuff(buffer, toSerialize.GetName());
        writeVarInt(buffer, 0);
        writeVarInt(buffer, 0);
    }
};

template<>
struct iu::Serializer<mc::server::StatusPacket>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::StatusPacket& toSerialize)
    {
        using namespace mc::util;
        writeVarInt(buffer, toSerialize.Size());
        writeVarInt(buffer, toSerialize.GetId<int>());
        writeStringToBuff(buffer, toSerialize.Json().dump());
    }
};

template<>
struct iu::Serializer<mc::server::FinishConfiguration>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::FinishConfiguration& toSerialize)
    {
        using namespace mc::util;
        writeVarInt(buffer, toSerialize.Size());
        writeVarInt(buffer, toSerialize.GetId<int>());
    }
};

template<>
struct iu::Serializer<mc::server::LoginPlayPacket>
{
    size_t GetSize(const mc::server::LoginPlayPacket& object) { return object.Size();}

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

        writeVarInt(buffer, toSerialize.GetId<int>());
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
        writeVarInt(buffer, toSerialize.m_dimensionType);
        identifierSerializer.Serialize(buffer, toSerialize.m_dimensionName);
        int64Serializer.Serialize(buffer, toSerialize.m_seedHash);
        byteSerialier.Serialize(buffer, toSerialize.m_gameMode);
        byteSerialier.Serialize(buffer, toSerialize.m_previousGameMode);
        boolSerializer.Serialize(buffer, toSerialize.m_isDebug);
        boolSerializer.Serialize(buffer, toSerialize.m_isFlat);
        byteSerialier.Serialize(buffer, toSerialize.m_deathDimension.has_value() && toSerialize.m_deathPosition.has_value());

        if(toSerialize.m_deathDimension.has_value() && toSerialize.m_deathPosition.has_value())
        {
            identifierSerializer.Serialize(buffer, toSerialize.m_deathDimension.value().AsString());
            positionSerializer.Serialize(buffer, toSerialize.m_deathPosition.value());
        }

        writeVarInt(buffer, toSerialize.m_portalCooldown);
        byteSerialier.Serialize(buffer, toSerialize.m_enforceSecureChat);
        writeVarInt(buffer, 0, toSerialize.Size());
    }
};

template<>
struct iu::Serializer<mc::server::GameEvent>
{
    size_t GetSize(const mc::server::GameEvent& object)
    {
        return object.Size();
    }

    void Serialize(std::vector<uint8_t>& buffer, const mc::server::GameEvent& toSerialize)
    {
        using namespace mc::util;

        writeVarInt(buffer, GetSize(toSerialize));
        writeVarInt(buffer, toSerialize.GetId<int>());
        ByteSerializer().Serialize(buffer, (std::uint8_t)toSerialize.m_event);
        FloatSerializer().Serialize(buffer, toSerialize.m_value);
    }
};


template<>
struct iu::Serializer<mc::server::SynchronisePlayerPosition>
{
    size_t GetSize(const mc::server::SynchronisePlayerPosition& object)
    {
        return object.Size();
    }

    void Serialize(std::vector<uint8_t>& buffer, const mc::server::SynchronisePlayerPosition& toSerialize)
    {
        using namespace mc::util;

        writeVarInt(buffer, GetSize(toSerialize));
        writeVarInt(buffer, toSerialize.GetId<mc::NBT::Int>());
        DoubleSerializer().Serialize(buffer, toSerialize.m_x);
        DoubleSerializer().Serialize(buffer, toSerialize.m_y);
        DoubleSerializer().Serialize(buffer, toSerialize.m_z);
        FloatSerializer().Serialize(buffer, toSerialize.m_yaw);
        FloatSerializer().Serialize(buffer, toSerialize.m_pitch);
        ByteSerializer().Serialize(buffer, toSerialize.m_relativeMask);
        writeVarInt(buffer, toSerialize.m_teleportID);
    }
};

#endif // SERVER_PACKETS_H