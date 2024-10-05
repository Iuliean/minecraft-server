#include <SFW/Server.h>
#include <SFW/Logger.h>
#include <SFW/LoggerManager.h>
#include <cstdint>
#include "BlockState.h"
#include "DataTypes/Identifier.h"
#include "MinecraftHandler.h"
#include "DataTypes/nbt.h"
#include <fstream>
#include <unordered_map>
#include "Registry.h"
#include "zstr.hpp"
int main()
{
    iu::LoggerManager::Init();
    mc::BlockStateRegistry::Init("blocks.json");
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
    auto& log = iu::LoggerManager::GlobalLogger();
    mc::BlockState s(mc::Identifier("acacia_button"));
    s.AddProperty("face", "floor");
    s.AddProperty("facing", "north");
    s.AddProperty("powered", "true");
    std::cout << mc::BlockStateRegistry::Instance().GetBlockStateId(s).value() << '\n' ;

#endif
    mc::BlockStateRegistry::Deinit();
    return 0;
}