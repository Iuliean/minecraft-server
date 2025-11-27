#ifndef MINECRAFT_HANDLER_HPP
#define MINECRAFT_HANDLER_HPP
#include <SFW/ServerConnectionHandler.h>
#include <SFW/Connection.h>

#include <atomic>

#include "server_context.hpp"
#include "server_state.hpp"
#include "packet_dispatcher.hpp"
namespace mc
{

    class MinecraftHanlder: public iu::ServerConnectionHandler, packet_dispatcher
    {
    public:
        MinecraftHanlder();
        virtual ~MinecraftHanlder()= default;

        void HandleConnection(iu::Connection& connection)override;
        void OnConnected(iu::Connection& connection)override;
        void Stop()override;

    private:

        void dispatch() override;

        server_state m_state;
        ServerContext m_context;
        std::atomic_bool m_stop;
    };
}

#endif //MINECRAFT_HANDLER_H
