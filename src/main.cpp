#include <Server.h>
#include <cstdint>
#include <iostream>

#include "Logger.h"
#include "LoggerManager.h"
#include "MinecraftHandler.h"

#include "NBT/nbt.h"
#include "spdlog/fmt/bundled/core.h"

void func(const mc::NBT::NamedIntArray& obj)
{
    iu::LoggerManager::GlobalLogger().info("test");
}

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
    */
#else
    //"advanced" testing for NBT tags
    std::vector<int> v;
    v.push_back(2);
    v.push_back(23);
    mc::NBT::NamedIntArray s(v, "arr");
    mc::NBT::NamedIntArray s2 = std::move(s);

    func(std::move(s2));
    
#endif
    return 0;
}