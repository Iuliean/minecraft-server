#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H
#include <vector>
#include <array>
#include <stdint.h>
#include "DataTypes/nbt.h"


namespace mc
{
    using ChunkRegion = std::array<std::array<NBT::NBT, 32>, 32>;

    struct ServerContext
    {
        //Registry packets are prebuilt from the json
        std::array<std::vector<std::uint8_t>, 8> registry_packets;
        ChunkRegion chunk_region;
    };
}

#endif //SERVER_CONTEXT_H