#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "SFW/Connection.h"
#include "data_types/uuid.hpp"
#include "packet_dispatcher.hpp"
#include "player_handler.hpp"
#include "server_context.hpp"
#include "server_state.hpp"
#include <unordered_map>

namespace mc
{

    class instance
    {
    public:

        void register_player(const uuid& uuid, iu::Connection& client, const ServerContext& ctx, packet_dispatcher& dispatcher, server_state& state);

    private:
        void main_loop();

        std::unordered_map<uuid, player_handler> m_players;
    };
}

#endif //INSTANCE_HPP