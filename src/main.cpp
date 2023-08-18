#include <Server.h>
#include <iostream>

#include "Logger.h"
#include "LoggerManager.h"
#include "MinecraftHandler.h"
int main()
{

    iu::LoggerManager::Init();
    iu::LoggerManager::SetLevel(iu::LogLevel::DEBUG);

    iu::AggregateServer<mc::MinecraftHanlder> server("0.0.0.0", 25565);
    std::thread serverThread([&server](){
        server.Run();
    });
    char c = 'a'; 
    std::cin >> c;

    server.Stop();

    serverThread.join();

    return 0;
}