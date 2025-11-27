#include <SFW/LoggerManager.h>
#include <SFW/Connection.h>
#include <bit>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <filesystem>
#include <string>
#include <sys/types.h>
#include <fstream>


#include "MinecraftHandler.h"
#include "ClientPackets.h"
#include "DataTypes/nbt.h"
#include "zstr.hpp"

namespace mc
{

    namespace
    {
        constexpr static int PACKET_SIZE = 10024;
        //Temporary hopefully
        [[maybe_unused]]
        ChunkRegion loadChunkRegion(std::filesystem::path path)
        {

            ChunkRegion out;
            std::ifstream data(path);
            std::array<int, 1024> chunkOffsets;
            [[maybe_unused]]
            constexpr size_t  a = 4 * ((0 & 31) + (0 & 31) * 32);
            data.read((char*)chunkOffsets.data(), sizeof(chunkOffsets));
            data.seekg(0);
            for (const int offsetAndSize : chunkOffsets)
            {
                const size_t offset = int(std::byteswap(int(offsetAndSize & 0x00ffffff)) >> 8) * 4096;
                const size_t chunk_size = (offsetAndSize & 0xff000000) >> 24;

                if (offset == 0 && chunk_size == 0) continue;

                SFW_LOG_DEBUG("chunkLoading", "Loading chunk at offset:{:#x} with length:{} * 4kb sectors", offset, chunk_size);
                data.seekg(offset + 5, data.beg);

                zstr::istreambuf decompBuff(data.rdbuf());
                std::istream stream(&decompBuff);

                nbt::nbt chunk = nbt::parse(stream);

                const int x = chunk["xPos"];
                const int z = chunk["zPos"];
                SFW_LOG_INFO("chunkLoading","Chunk x{}, z{}", x, z);

                out[x][z].emplace(std::move(chunk));
            }
            return out;
        }
    }

    MinecraftHanlder::MinecraftHanlder()
        : ServerConnectionHandler(),
          m_state(),
          m_stop(false)
    {
        m_context.chunk_region = loadChunkRegion("map/r.0.0.mca");
    }

    void MinecraftHanlder::OnConnected(iu::Connection& connection)
    {
        SFW_LOG_INFO("MinecraftHandler", "New connection from: {}:{}", connection.GetAdress(), connection.GetPort());
    }

    void MinecraftHanlder::HandleConnection(iu::Connection &connection)
    {
        try
        {
            std::array<uint8_t, PACKET_SIZE> data;

            const size_t recieved_size = connection.Receive(data);
            if (recieved_size == 0)
            {
                SFW_LOG_INFO("MinecraftHandler", "No data received from {}:{}", connection.GetAdress(), connection.GetPort());
                return;
            }

        }
        catch(const std::exception& e)
        {
            SFW_LOG_ERROR("MinecraftHandler", "Failed to handle connection: {}", e.what());
        }
    }

    void MinecraftHanlder::dispatch()
    {

    }

    void MinecraftHanlder::Stop()
    {
        return;
    }


}
