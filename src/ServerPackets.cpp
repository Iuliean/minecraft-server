#include "ServerPackets.h"

#include "ClientPackets.h"
#include "DataTypes/Identifier.h"
#include "Packet.h"
#include "utils.h"

namespace mc::server
{
    // ****************
    // * LoginPackets *
    // ****************
    LoginSuccessPacket::LoginSuccessPacket(const client::LoginStartPacket& packet)
        : Packet(LoginPacketID::SUCCESS),
          m_uuid(packet.GetUUID()),
          m_name(packet.GetPlayerName()),
          m_numOfElements(0),
          m_strictErrorHandling(true)
    {
    }
    // *****************
    // * StatusPackets *
    // *****************
    StatusPacket::StatusPacket() : Packet(StatusPacketID::STATUS)
    {
        // FORMATED
        m_payload = { { "version", { { "name", "1.20.6" }, { "protocol", 766 } } },
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
    // *****************
    // * ConfigPackets *
    // *****************
    FinishConfiguration::FinishConfiguration()
        : Packet((int)ConfigPacketID::FinishConfiguration)
    {
    }

    // ****************
    // * PlayPackets *
    // ****************
    LoginPlayPacket::LoginPlayPacket()
        : Packet((int)PlayPacketID::LoginPlay),
        m_entityID(243645754),
        m_isHardcore(false),
        m_dimensionIdentifiers({Identifier("overworld"), Identifier("nether")}),
        m_maxPlayers(32),
        m_viewDistance(16),
        m_simulationDistance(16),
        m_reducedDebugInfo(false),
        m_enableRespawnScreen(true),
        m_limitedCrafting(false),
        m_dimensionType(0),
        m_dimensionName(Identifier("overworld")),
        m_seedHash(12312312),
        m_gameMode(3),
        m_previousGameMode(-1),
        m_isDebug(false),
        m_isFlat(false),
        m_hasDeathLocation(false),
        m_deathDimension(),
        m_deathPosition(),
        m_portalCooldown(1),
        m_enforceSecureChat(false)
        {
        }

    GameEvent::GameEvent(GameEvent::Event event, float value)
        : m_event(event),
        m_value(value),
        Packet((int)PlayPacketID::GameEvent)
    {
    }

    SynchronisePlayerPosition::SynchronisePlayerPosition(double x, double y, double z, float yaw, float pitch, std::uint8_t relativeMask)
        : Packet((int)PlayPacketID::SynchronisePlayerPosition),
        m_x(x),
        m_y(y),
        m_z(z),
        m_yaw(yaw),
        m_pitch(pitch),
        m_relativeMask(relativeMask),
        m_teleportID(123)
    {
    }
} // namespace mc::server