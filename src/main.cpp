#include <Server.h>
#include <cstdint>
#include <iostream>
#include <Position.h>
#include "Logger.h"
#include "LoggerManager.h"
#include "MinecraftHandler.h"

#include "spdlog/fmt/bundled/core.h"

int main()
{
    iu::LoggerManager::Init();
    iu::LoggerManager::SetLevel(iu::LogLevel::DEBUG);

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
    mc::Position p(-123, 2, 222);
    iu::LoggerManager::GlobalLogger().info("X:{} Z:{} Y:{}", p.GetX(), p.GetZ(), p.GetY());
    p.SetX(12352).SetY(333).SetZ(123);
    iu::LoggerManager::GlobalLogger().info("X:{} Z:{} Y:{}", p.GetX(), p.GetZ(), p.GetY());

    
#endif
    return 0;
}