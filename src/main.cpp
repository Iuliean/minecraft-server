#include <SFW/Server.h>
#include <SFW/Logger.h>
#include <SFW/LoggerManager.h>
#include <cstdint>
#include "MinecraftHandler.h"
#include "DataTypes/nbt.h"
#include <fstream>
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
    auto& log = iu::LoggerManager::GlobalLogger();
    using namespace mc::NBT;
    mc::NBT::NBT objects("Root");
    objects->Insert("byte", (Byte)-1);
    objects->Insert("Short", (Short)300);
    objects->Insert("Int", 123123132);
    objects->Insert("Looong", (Long)123123);
    objects->Insert("float", (Float)1.5);
    objects->Insert("double", (Double)1.000000001010);
    objects->Insert("byte_array", ByteArray{1,2,3,4,5,6,7,8,-1,-2});
    objects->Insert("string", String{"hello this is my stirng"});
    objects->Insert(NamedCompound("compound"));
    objects->Insert(NamedList("list"));
    objects->Insert("int_arat", IntArray{1,2,3,4,5,6,7,1});
    objects->Insert("long_array", LongArray{123123124515236346,1231241253252363437});
    objects->Get<NBTList>("list")->Insert(123);

    std::vector<uint8_t> out;

    iu::Serializer<NamedCompound>().Serialize(out, objects);

    std::ofstream fout("out.dat", std::ios::binary);
    for(auto b : out)
        fout << b;

    fout.flush();
    fout.close();

    std::ifstream f("out.dat");
    auto newObj = parse(std::move(f));

    for (const auto& [name, tag] : newObj.Get())
    {
        iu::LoggerManager::GlobalLogger().debug("{}: {}", name, *tag);
    }
#endif
    return 0;
}