#include <Server.h>
#include <iostream>

#include "utils.h"
#include "MinecraftHandler.h"
#include "spdlog/common.h"

int main()
{
    iu::AggregateServer<mc::MinecraftHanlder> server("0.0.0.0", 25565);
    server.SetLogLevel(spdlog::level::debug);

    std::thread serverThread([&server](){
        server.Run();
    });
    char c = 'a'; 
    std::cin >> c;

    server.Stop();

    serverThread.join();

    return 0;
}