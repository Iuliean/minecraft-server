#include "ServerPackets.h"

#include "ClientPackets.h"
#include "DataTypes/Identifier.h"
#include "Packet.h"
#include "utils.h"

namespace mc::server
{

    LoginSuccessPacket::LoginSuccessPacket(const client::LoginStartPacket& packet)
        : Packet(LoginPacketID::SUCCESS),
          m_uuid(packet.GetUUID()),
          m_name(packet.GetPlayerName()),
          m_numOfElements(0),
          m_strictErrorHandling(true)
    {
    }

    StatusPacket::StatusPacket() : Packet(StatusPacketID::STATUS)
    {
        // FORMATED
        m_payload = { { "version", { { "name", "1.20.4" }, { "protocol", 767 } } },
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
        m_gameMode(1),
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

} // namespace mc::server