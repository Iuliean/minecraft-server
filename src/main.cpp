#include <SFW/Server.h>
#include <SFW/Logger.h>
#include <SFW/LoggerManager.h>
#include <cstdint>
#include "DataTypes/nbt.h"

#include <fstream>
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
    auto& log = iu::LoggerManager::GlobalLogger();
    using namespace mc::NBT;
    NamedCompound objects("hello world");
    objects->Insert(NamedString("name", "Bananrama"));
    objects->Insert(NamedInt("somevalue", 123));
    objects->Insert(NamedCompound("ested"));
    objects->Get<NBTCompound>("ested")->Insert("hello", std::string("hhhh"));

    objects->Insert("Somelist", NBTList{})->Insert(12);
    auto& list = objects->Get<NBTList>("Somelist");
    list->Insert(323);
    list->Insert(24);
    list->Insert(14);
    list->Insert(44);

    objects->Insert(NamedString("Somestring", "am coaili mari si sunt bun rau"));
    std::vector<uint8_t> out;

    iu::Serializer<NamedCompound>().Serialize(out, objects);

    std::ofstream fs("out.dat", std::ios::binary);
    for(auto byte: out)
    {
        fs << byte;
    }
#endif
    return 0;
}