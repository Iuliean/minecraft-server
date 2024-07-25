#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H
#include <vector>
#include <array>
#include <stdint.h>


namespace mc
{
    struct ServerContext
    {
        //Registry packets are prebuilt from the json
        std::array<std::vector<std::uint8_t>, 8> registry_packets;
    };
}

#endif //SERVER_CONTEXT_H