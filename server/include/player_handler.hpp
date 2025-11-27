#ifndef PLAYER_HANDLER_HPP
#define PLAYER_HANDLER_HPP
#include <memory>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <SFW/Connection.h>

#include "packet.hpp"
#include "client_packets.hpp"
#include "SFW/LoggerManager.h"
#include "server_context.hpp"
#include "server_packets.hpp"
#include "utils.hpp"

namespace mc
{
    enum class PlayerHandlerState: int
    {
        IDLE    = 0,
        STATUS  = 1,
        LOGIN   = 2,
        CONFIG  = 3,
        PLAY    = 4
    };

    class PlayerHandler
    {
    public:
        PlayerHandler() = delete;
        PlayerHandler(const PlayerHandler&) = delete;
        PlayerHandler(PlayerHandler&&) = delete;

        PlayerHandler& operator=(const PlayerHandler&) = delete;
        PlayerHandler& operator=(PlayerHandler&&) = delete;

        PlayerHandler(iu::Connection& client, const ServerContext& context);
        ~PlayerHandler() = default;

        //void Execute(const std::vector<uint8_t>& data);

        void PlayLoop();

    private:

        iu::Connection& m_client;
        PlayerHandlerState m_state;
        const ServerContext& m_context;
        server::status_packet m_statusMessage;
    };
}
#endif //PLAYER_HANDLER_H
