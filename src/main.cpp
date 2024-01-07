#include <SFW/Server.h>
#include <SFW/Logger.h>
#include <SFW/LoggerManager.h>
#include "DataTypes/nbt.h"


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
    NamedCompound objects("Root");
    NamedInt intnamed("myint", 23);
    NamedIntArray intarr("MyArr", {1,2,3,4,5,6,7});
    NamedList list("list");
    list.Get().Add(UnnamedInt{23});
    list.Get().Add(UnnamedInt{2444});
    objects.Get().Add(intnamed);
    objects.Get().Add(intarr);
    objects.Get().Add(list);

    for (const auto& [name, value] : objects.Get())
    {
        if (name == "myint")
        {
            log.info("{}", *dynamic_cast<NamedInt*>(value));
        }
        if(name == "MyArr")
        {
            log.info("{}", *dynamic_cast<NamedIntArray*>(value));
        }
        if(name == "list")
        {
            auto& l = dynamic_cast<NamedList*>(value)->Get();
            for (auto it = l.begin(); it < l.end(); it++)
            {
                log.info("{}", it.get<int>());
            }
        }
    }

#endif
    return 0;
}