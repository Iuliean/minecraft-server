#ifndef SERVER_PACKETS_HPP
#define SERVER_PACKETS_HPP

#include "client_packets.hpp"
#include "data_types/identifier.hpp"
#include "data_types/uuid.hpp"
#include "packet.hpp"
#include "data_types/position.hpp"
#include <nlohmann/json.hpp>
#include "SFW/Serializer.h"
#include "utils.hpp"

#include <cstdint>
#include <string>
#include <unistd.h>
#include <utility>
namespace mc::server
{

    enum class idle_packet_id : int
    {
        unknown = -1
    };

    enum class status_packet_id : int
    {
        unknown = -1,
        status  = 0
    };

    enum class login_packet_id : int
    {
        unknown = -1,
        success = 0x02
    };

    enum class config_packet_id : int
    {
        unknown = -1,
        finish_config = 0x03,
        known_packs = 0x0E
    };

    enum class play_packet_id : int
    {
        unknown   = -1,
        game_event = 0x22,
        login_play = 0x2b,
        sync_player_pos = 0x41
    };

    // ****************
    // * LoginPackets *
    // ****************

    class login_success_packet : public packet
    {
    public:
        login_success_packet(const client::login_start_packet& packet)
            : mc::packet(login_packet_id::success),
              m_uuid(packet.get_uuid()),
              m_name(packet.get_player_name()),
              m_elements_count(0)
        {}
        virtual ~login_success_packet() = default;

        constexpr uuid get_uuid() const noexcept { return m_uuid; }
        constexpr const std::string& get_name()const noexcept { return m_name; }

        std::string as_string() const override
        {
            return std::format("name: {}, uuid: {}", m_name, m_uuid);
        }

        constexpr std::string packet_name() const override { return "login_success_packet"; }
        size_t size() const override
        {
            return packet::size() +
                   sizeof(uuid) +
                   util::sizeOfString(m_name) +
                   util::sizeOfVarInt(m_elements_count);
        }
    private:
        uuid m_uuid;
        std::string m_name;
        util::varInt m_elements_count;
    };

    // *****************
    // * StatusPackets *
    // *****************

    class status_packet : public packet
    {
    public:
        status_packet()
            : packet(status_packet_id::status),
              m_payload()
        {
            // FORMATED
            m_payload = { { "version", { { "name", "1.21.8" }, { "protocol", 772 } } },
                { "players",
                    { { "max", 10 },
                        { "online", 0 },
                        { "sample",
                            { { "name", "thinkofdeath" },
                                { "id", "4566e69f-c907-48ee-8d71-d7ba5aa00d20" } } } } },
                { "description",
                    { { "text",
                        "This is a shit implementation of an Mc server that doesn't even work" } } },
                { "enforceSecureChat", true },
                { "previewsChat", true } };
        }
        virtual ~status_packet() = default;

        decltype(auto) json(this auto&& self) noexcept { return self.m_payload; }

        std::string as_string() const override { return std::format("{}", m_payload); }
        constexpr std::string packet_name() const override { return "status_packet"; }

        size_t size() const override { return packet::size() + util::sizeOfString(m_payload.dump());}

    private:
        nlohmann::json m_payload;
    };
    // *****************
    // * ConfigPackets *
    // *****************

    class known_packs : public packet
    {
    public:
        known_packs(std::string nspace, std::string id, std::string version)
        : packet(std::to_underlying(config_packet_id::known_packs)),
          m_namespace(std::move(nspace)),
          m_id(std::move(id)),
          m_version(std::move(version))
        {
        }
        virtual ~known_packs() = default;

        std::string as_string()const override
        {
            return std::format("namespace:{}, id:{}, version:{}", m_namespace, m_id, m_version);
        }

        constexpr std::string packet_name()const override { return "known_packs"; }

        size_t size()const override
        {
            return packet::size() +
                   util::sizeOfString(m_namespace) +
                   util::sizeOfString(m_id) +
                   util::sizeOfString(m_version);
        }
    private:
        friend iu::Serializer<known_packs>;
        std::string m_namespace;
        std::string m_id;
        std::string m_version;
    };

    class finish_config : public packet
    {
    public:
        finish_config()
            : packet(config_packet_id::finish_config)
        {
        }
        virtual ~finish_config() = default;

        std::string as_string() const override { return ""; }
        constexpr std::string packet_name() const override { return "finish_config";}
        size_t size() const override { return packet::size();}
    };


    // ****************
    // * PlayPackets *
    // ****************

    class login_play_packet : public packet
    {
    public:
        login_play_packet()
        : mc::packet(play_packet_id::login_play),
          m_entity_id(243645754),
          m_is_hardcore(false),
          m_dimension_identifiers({Identifier("overworld"), Identifier("nether")}),
          m_max_players(32),
          m_view_distance(16),
          m_simulation_distance(16),
          m_reduced_debug_info(false),
          m_enable_respawn_screen(true),
          m_limited_crafting(false),
          m_dimention_type(0),
          m_dimension_name(Identifier("overworld")),
          m_seed_hash(12312312),
          m_game_mode(1),
          m_previous_game_mode(-1),
          m_is_debug(false),
          m_is_flat(false),
          m_has_death_location(false),
          m_death_dimenstion(),
          m_death_position(),
          m_portal_cooldown(1),
          m_sea_level(100),
          m_enforce_secure_chat(false)
        {
        }
        virtual ~login_play_packet() = default;

        std::string as_string() const override
        {
            constexpr const char* const text =
                "EntityID:{}, "
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
                "portalCooldown:{}"
                "seaLevel:{}";
            return std::format(text,
                m_entity_id,
                m_is_hardcore,
                m_max_players,
                m_view_distance,
                m_simulation_distance,
                m_reduced_debug_info,
                m_enable_respawn_screen,
                m_limited_crafting,
                m_seed_hash,
                m_game_mode,
                m_previous_game_mode,
                m_is_debug,
                m_is_flat,
                m_has_death_location,
                m_sea_level,
                m_portal_cooldown);
        }

        constexpr std::string packet_name() const override { return "login_play"; }
        size_t size() const override
        {
            const size_t deathLocationSize =
                m_has_death_location ? (util::sizeOfString(m_death_dimenstion->AsString()) + sizeof(*m_death_dimenstion)) : 0;

            size_t idendtifiers_size = util::sizeOfVarInt(m_dimension_identifiers.size());
            for (const auto& identifier: m_dimension_identifiers)
                idendtifiers_size += util::sizeOfString(identifier.AsString());

            return packet::size() + //ID
                sizeof(m_entity_id) +
                sizeof(m_is_hardcore) +
                idendtifiers_size +
                util::sizeOfVarInt(m_max_players) +
                util::sizeOfVarInt(m_view_distance) +
                util::sizeOfVarInt(m_simulation_distance) +
                sizeof(m_reduced_debug_info) +
                sizeof(m_enable_respawn_screen) +
                sizeof(m_limited_crafting) +
                util::sizeOfVarInt(m_dimention_type) +
                util::sizeOfString(m_dimension_name.AsString()) +
                sizeof(m_seed_hash) +
                sizeof(m_game_mode) +
                sizeof(m_previous_game_mode) +
                sizeof(m_is_debug) +
                sizeof(m_is_flat) +
                sizeof(m_has_death_location) +
                deathLocationSize +
                util::sizeOfVarInt(m_portal_cooldown) +
                util::sizeOfVarInt(m_sea_level) +
                sizeof(m_enforce_secure_chat);
        }

    private:
        friend iu::Serializer<mc::server::login_play_packet>;

        int32_t m_entity_id;
        bool m_is_hardcore;
        std::vector<mc::Identifier> m_dimension_identifiers;
        util::varInt m_max_players;
        util::varInt m_view_distance;
        util::varInt m_simulation_distance;
        bool m_reduced_debug_info;
        bool m_enable_respawn_screen;
        bool m_limited_crafting;
        util::varInt m_dimention_type;
        Identifier m_dimension_name;
        int64_t m_seed_hash;
        uint8_t m_game_mode;
        int8_t m_previous_game_mode;
        bool m_is_debug;
        bool m_is_flat;
        bool m_has_death_location;
        std::optional<Identifier> m_death_dimenstion;
        std::optional<position> m_death_position;
        util::varInt m_portal_cooldown;
        util::varInt m_sea_level;
        bool m_enforce_secure_chat;
    };

    class game_event : public packet
    {
    public:
        enum class event : std::uint8_t
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
        game_event(event event, float value = 0) noexcept
            : packet(play_packet_id::game_event),
              m_event(event),
              m_value(value)
        {}
        virtual ~game_event() = default;

        std::string as_string() const override { return std::format("Event: {}, Value: {}", std::to_underlying(m_event), m_value); }
        constexpr std::string packet_name() const override { return "game_event"; }
        constexpr size_t size() const override { return packet::size() + sizeof(event) + sizeof(float); }
    private:
        friend iu::Serializer<mc::server::game_event>;
        event m_event;
        float m_value; //depends on event
    };

    class sync_player_position : public packet
    {
    public:
        sync_player_position(
            util::varInt teleport_id,
            double x,
            double y,
            double z,
            double velocity_x,
            double velocity_y,
            double velocity_z,
            float yaw,
            float pitch,
            int relativeMask
        )
        : packet(play_packet_id::sync_player_pos),
          m_teleport_id(teleport_id),
          m_x(x),
          m_y(y),
          m_z(z),
          m_velocity_x(velocity_x),
          m_velocity_y(velocity_y),
          m_velocity_z(velocity_z),
          m_yaw(yaw),
          m_pitch(pitch),
          m_relative_mask(relativeMask)

        {}
        virtual ~sync_player_position() = default;

        std::string as_string() const override
        {
            constexpr auto fmt =
                "X: {}, Y: {}, Z: {}, Yaw: {}, Pitch: {},"
                "RelativeMask:{:b}, TeleportID: {}";
            return std::format(fmt,
                m_x, m_y, m_z, m_yaw, m_pitch,
                m_relative_mask, m_teleport_id
            );
        }

        constexpr std::string packet_name() const override { return "sync_player_position"; }
        size_t size() const override
        {
            return packet::size() + //ID
                   util::sizeOfVarInt(m_teleport_id) +
                   sizeof(m_x) +
                   sizeof(m_y) +
                   sizeof(m_z) +
                   sizeof(m_velocity_x) +
                   sizeof(m_velocity_y) +
                   sizeof(m_velocity_z) +
                   sizeof(m_yaw) +
                   sizeof(m_pitch) +
                   sizeof(m_relative_mask);
        }
    private:
        friend iu::Serializer<mc::server::sync_player_position>;

        util::varInt m_teleport_id;

        double m_x;
        double m_y;
        double m_z;

        double m_velocity_x;
        double m_velocity_y;
        double m_velocity_z;

        float m_yaw;
        float m_pitch;

        int m_relative_mask;
    };

} // namespace mc::server

template<>
struct iu::Serializer<mc::server::login_success_packet>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::login_success_packet& toSerialize)
    {
        using namespace mc::util;
        iu::Serializer<mc::uuid> uuidSerializer;

        writeVarInt(buffer, toSerialize.size());
        writeVarInt(buffer, toSerialize.get_id<int>());
        uuidSerializer.Serialize(buffer, toSerialize.get_uuid());
        writeStringToBuff(buffer, toSerialize.get_name());
        writeVarInt(buffer, 0);
    }
};

template<>
struct iu::Serializer<mc::server::status_packet>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::status_packet& toSerialize)
    {
        using namespace mc::util;
        writeVarInt(buffer, toSerialize.size());
        writeVarInt(buffer, toSerialize.get_id<int>());
        writeStringToBuff(buffer, toSerialize.json().dump());
    }
};

template<>
struct iu::Serializer<mc::server::finish_config>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::finish_config& toSerialize)
    {
        using namespace mc::util;
        writeVarInt(buffer, toSerialize.size());
        writeVarInt(buffer, toSerialize.get_id<int>());
    }
};

template<>
struct iu::Serializer<mc::server::login_play_packet>
{
    size_t GetSize(const mc::server::login_play_packet& object) { return object.size();}

    void Serialize(std::vector<uint8_t>& buffer, const mc::server::login_play_packet& toSerialize)
    {
        using namespace mc::util;

        IntSerializer intSerializer;
        LongSerializer int64Serializer;
        UnsignedCharSerializer byteSerialier;
        BoolSerializer boolSerializer;
        Serializer<mc::Identifier> identifierSerializer;
        Serializer<mc::position> positionSerializer;
        Serializer<std::vector<mc::Identifier>> identifierVecSerializer;

        writeVarInt(buffer, toSerialize.size());
        writeVarInt(buffer, toSerialize.get_id<int>());
        intSerializer.Serialize(buffer, toSerialize.m_entity_id);
        boolSerializer.Serialize(buffer, toSerialize.m_is_hardcore);
        writeVarInt(buffer, toSerialize.m_dimension_identifiers.size());
        identifierVecSerializer.Serialize(buffer, toSerialize.m_dimension_identifiers);
        writeVarInt(buffer, toSerialize.m_max_players);
        writeVarInt(buffer, toSerialize.m_view_distance);
        writeVarInt(buffer, toSerialize.m_simulation_distance);
        boolSerializer.Serialize(buffer, toSerialize.m_reduced_debug_info);
        boolSerializer.Serialize(buffer, toSerialize.m_enable_respawn_screen);
        boolSerializer.Serialize(buffer, toSerialize.m_limited_crafting);
        writeVarInt(buffer, toSerialize.m_dimention_type);
        identifierSerializer.Serialize(buffer, toSerialize.m_dimension_name);
        int64Serializer.Serialize(buffer, toSerialize.m_seed_hash);
        byteSerialier.Serialize(buffer, toSerialize.m_game_mode);
        byteSerialier.Serialize(buffer, toSerialize.m_previous_game_mode);
        boolSerializer.Serialize(buffer, toSerialize.m_is_debug);
        boolSerializer.Serialize(buffer, toSerialize.m_is_flat);
        boolSerializer.Serialize(buffer, toSerialize.m_death_dimenstion.has_value() && toSerialize.m_death_position.has_value());

        if(toSerialize.m_death_dimenstion.has_value() && toSerialize.m_death_position.has_value())
        {
            identifierSerializer.Serialize(buffer, toSerialize.m_death_dimenstion.value().AsString());
            positionSerializer.Serialize(buffer, toSerialize.m_death_position.value());
        }

        writeVarInt(buffer, toSerialize.m_portal_cooldown);
        writeVarInt(buffer, toSerialize.m_sea_level);
        boolSerializer.Serialize(buffer, toSerialize.m_enforce_secure_chat);
    }
};

template<>
struct iu::Serializer<mc::server::game_event>
{
    size_t GetSize(const mc::server::game_event& object)
    {
        return object.size();
    }

    void Serialize(std::vector<uint8_t>& buffer, const mc::server::game_event& toSerialize)
    {
        using namespace mc::util;

        writeVarInt(buffer, GetSize(toSerialize));
        writeVarInt(buffer, toSerialize.get_id<int>());
        UnsignedCharSerializer().Serialize(buffer, (std::uint8_t)toSerialize.m_event);
        FloatSerializer().Serialize(buffer, toSerialize.m_value);
    }
};


template<>
struct iu::Serializer<mc::server::sync_player_position>
{
    size_t GetSize(const mc::server::sync_player_position& object)
    {
        return object.size();
    }

    void Serialize(std::vector<uint8_t>& buffer, const mc::server::sync_player_position& toSerialize)
    {
        using namespace mc::util;

        writeVarInt(buffer, GetSize(toSerialize));
        writeVarInt(buffer, toSerialize.get_id<int>());
        writeVarInt(buffer, toSerialize.m_teleport_id);
        DoubleSerializer().Serialize(buffer, toSerialize.m_x);
        DoubleSerializer().Serialize(buffer, toSerialize.m_y);
        DoubleSerializer().Serialize(buffer, toSerialize.m_z);
        DoubleSerializer().Serialize(buffer, toSerialize.m_velocity_x);
        DoubleSerializer().Serialize(buffer, toSerialize.m_velocity_y);
        DoubleSerializer().Serialize(buffer, toSerialize.m_velocity_z);
        FloatSerializer().Serialize(buffer, toSerialize.m_yaw);
        FloatSerializer().Serialize(buffer, toSerialize.m_pitch);
        IntSerializer().Serialize(buffer, toSerialize.m_relative_mask);
    }
};

template<>
struct iu::Serializer<mc::server::known_packs>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::server::known_packs& toSerialize)
    {
        using namespace mc::util;
        const int packetSize = 2 +
                                  sizeOfString(toSerialize.m_namespace) +
                                  sizeOfString(toSerialize.m_id) +
                                  sizeOfString(toSerialize.m_version);

        writeVarInt(buffer, packetSize);
        writeVarInt(buffer, toSerialize.get_id<int>());
        writeVarInt(buffer, 1);
        writeStringToBuff(buffer, toSerialize.m_namespace);
        writeStringToBuff(buffer, toSerialize.m_id);
        writeStringToBuff(buffer, toSerialize.m_version);
    }
};

#endif // SERVER_PACKETS_H
