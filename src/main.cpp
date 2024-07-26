#include <SFW/Server.h>
#include <SFW/Logger.h>
#include <SFW/LoggerManager.h>
#include <cstdint>
#include "MinecraftHandler.h"
#include "DataTypes/nbt.h"
#include <fstream>
#include "zstr.hpp"
int main()
{
    iu::LoggerManager::Init();
    iu::LoggerManager::SetLevel(iu::LogLevel::DEBUG);

#if 1
    iu::AggregateServer<mc::MinecraftHanlder> server("192.168.0.31", 25565);
    std::thread serverThread([&server](){
        server.Run();
    });
    char c = 'a'; 
    std::cin >> c;

    server.Stop();

    serverThread.join();

#else 
    auto& log = iu::LoggerManager::GlobalLogger();
    std::ifstream data("r.0.0.mca");

    size_t offset = 4 * ((1 & 31) + (1 & 31) * 32);
    data.seekg(offset);
    unsigned char location[4];
    data.read((char*)&location, 4);

    int oset = 0;
    oset |= location[0];
    oset <<= 8;
    oset |= location[1];
    oset <<= 8;
    oset |= location[2];
    log.info("{:x} {:x} {:x} {:x}",location[0],location[1],location[2],location[3]);
    log.info("{}",oset);

    data.seekg((oset * 4096)+1, data.beg);
    log.info("{:x}", (int)data.tellg());
    char lenght [4];
    char comp_Type = 0;
    data.get((char*)&lenght,4);
    data.get(comp_Type);
    log.info("{:x}", (int)data.tellg());

    int len = 0;
    len |= lenght[0];
    len <<= 8;
    len |= lenght[1];
    len <<= 8;
    len |= lenght[2];
    len <<= 8;
    len |= lenght[3];
    //std::vector<std::uint8_t> chunk_info(len);
    //data.read((char*)chunk_info.data(), len-1);
    zstr::istreambuf lol(data.rdbuf(), 1 << 16, true);
    std::istream decompressed(&lol);
    auto nbt = mc::NBT::parse(decompressed);

    log.info("{} {}", nbt->Contains("Heightmaps"), nbt->Get<mc::NBT::String>("Status"));
#endif
    return 0;
}