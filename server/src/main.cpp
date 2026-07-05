#include <SFW/Server.h>
#include <SFW/LoggerManager.h>

#include "minecraft_handler.hpp"
#include "registry.hpp"

int main()
{
    iu::LoggerManager::LogToConsole();
    iu::LoggerManager::LogFile("lastrun.log");
#
    mc::BlockStateRegistry::Init("registries/blocks.json");
    iu::DistributedServer<mc::minecraft_handler> server("0.0.0.0", 25565);
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
