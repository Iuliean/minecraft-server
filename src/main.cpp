#include <Server.h>
#include <cstdint>
#include <iostream>
#include <Position.h>
#include "Logger.h"
#include "LoggerManager.h"
#include "MinecraftHandler.h"
#include "DataTypes/nbt.h"

#include "spdlog/fmt/bundled/core.h"
#include "spdlog/fmt/ranges.h"

int main()
{
    iu::LoggerManager::Init();
    iu::LoggerManager::SetLevel(iu::LogLevel::DEBUG);

#if 0
    iu::AggregateServer<mc::MinecraftHanlder> server("0.0.0.0", 25565);
    std::thread serverThread([&server](){
        server.Run();
    });
    char c = 'a'; 
    std::cin >> c;

    server.Stop();

    serverThread.join();

#else
    using namespace mc::NBT;
    auto& log = iu::LoggerManager::GlobalLogger();
    Byte v = 1;
    NamedByte byte("Hello",v);
    NamedInt byte2("Name", 234);
    NamedIntArray byte3("Name", {1,2,3});

    log.info("Byte1: {}", byte);
    log.info("Byte2: Value:{}", byte2);
    log.info("{}", byte3);

    NamedList list("MyList");
    
#endif
    return 0;
}