#ifndef MINECRAFT_HANDLER_H
#define MINECRAFT_HANDLER_H
#include <ServerConnectionHandler.h>
#include <Connection.h>
#include <memory>

namespace mc
{
    class MinecraftHanlder: public iu::ServerConnectionHandler
    {
    public:
        MinecraftHanlder();
        ~MinecraftHanlder()= default;

        void HandleConnection(iu::Connection& connection)override;
        void OnConnected(iu::Connection& connection)override;
        void Stop()override;
        
    private:
        std::shared_ptr<spdlog::logger> m_logger;
    };
}

#endif //MINECRAFT_HANDLER_H