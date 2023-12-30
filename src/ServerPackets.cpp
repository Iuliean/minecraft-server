#include "ServerPackets.h"
#include "ClientPackets.h"
#include "Packet.h"
#include "utils.h"

namespace mc
{
    namespace server
    {
        LoginSuccessPacket::LoginSuccessPacket(const client::LoginStartPacket& packet)
            : Packet(LoginPacketID::SUCCESS),
            m_uuid(packet.GetUUID()),
            m_name(packet.GetPlayerName()),
            m_numOfElements(0)
        {
        }

        StatusPacket::StatusPacket()
            : Packet(StatusPacketID::STATUS)
        {
            m_payload={
            {
                "version",
                    {
                        {"name","1.20.1"},
                        {"protocol",763}
                    }
                },
                {"players",
                    {
                        {"max", 10},
                        {"online", 0},
                        {"sample",
                            {
                                {"name", "thinkofdeath"},
                                {"id", "4566e69f-c907-48ee-8d71-d7ba5aa00d20"}
                            }
                        }
                    }
                },
                {"description",
                    {
                        {"text", "This is a shit implementation of an Mc server that doesn't even work"}
                    }
                },
                {"enforceSecureChat", true},
                {"previewsChat", true}
            };
        }
    }
}