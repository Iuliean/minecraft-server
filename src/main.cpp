#include <SFW/Server.h>
#include <SFW/LoggerManager.h>

#include "Registry.h"
#include "MinecraftHandler.h"

int main()
{
    iu::LoggerManager::LogToConsole();
    mc::BlockStateRegistry::Init("blocks.json");
#if 1
    iu::AggregateServer<mc::MinecraftHanlder> server("0.0.0.0", 25565);
    std::thread serverThread([&server](){
        server.Run();
    });
    char c = 'a'; 
    std::cin >> c;

    server.Stop();

    serverThread.join();

#else 

    //iu::AggregateServer<Test> server("0.0.0.0", 2222);
    //std::jthread s([&server](){ server.Run();});

    std::vector<uint8_t> lol;
    ByteSerializer().Serialize(lol, 12);

#endif
    mc::BlockStateRegistry::Deinit();
    return 0;
}