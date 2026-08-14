#include "instance.hpp"
#include "SFW/LoggerManager.h"
#include "player_handler.hpp"
#include <thread>
#include <tuple>
#include <utility>



namespace mc
{
    static constexpr auto DOM = "mc_instance";

    void instance::register_player(const uuid& uuid, iu::Connection& client, const ServerContext& context, packet_dispatcher& dispatcher, server_state& state)
    {
        SFW_LOG_DEBUG(DOM, "Registering player: {}", uuid);
        m_players.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(uuid),
            std::forward_as_tuple(
                client,
                context,
                dispatcher,
                state
            )
        );
    }

    void instance::main_loop()
    {
        using namespace std::chrono_literals;
        for(;;)
        {
            SFW_LOG_DEBUG("main_loop", "Tick...");

            std::this_thread::sleep_for(1s);
        }
    }
}