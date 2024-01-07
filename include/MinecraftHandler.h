#ifndef MINECRAFT_HANDLER_H
#define MINECRAFT_HANDLER_H
#include <SFW/LoggerManager.h>
#include <SFW/ServerConnectionHandler.h>
#include <SFW/Connection.h>
#include <atomic>

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