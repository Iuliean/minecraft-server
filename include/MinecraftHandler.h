#ifndef MINECRAFT_HANDLER_H
#define MINECRAFT_HANDLER_H
#include <LoggerManager.h>
#include <ServerConnectionHandler.h>
#include <Connection.h>
#include <atomic>
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
        iu::Logger m_logger;
        std::atomic_bool m_stop;
    };
}

#endif //MINECRAFT_HANDLER_H