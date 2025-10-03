#include <SFW/Server.h>
#include <SFW/LoggerManager.h>

#include "MinecraftHandler.h"
#include "Registry.h"

int main()
{
    iu::LoggerManager::LogToConsole();
    iu::LoggerManager::LogFile("lastrun.log");
#
    mc::BlockStateRegistry::Init("registries/blocks.json");
    iu::AggregateServer<mc::MinecraftHanlder> server("0.0.0.0", 25565);
    std::thread serverThread([&server](){
        server.Run();
    });
    char c = 'a'; 
    std::cin >> c;

    server.Stop();

    serverThread.join();

    mc::BlockStateRegistry::Deinit();

    return 0;
}