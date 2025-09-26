#include <SFW/LoggerManager.h>
#include <SFW/Connection.h>
#include <algorithm>
#include <bits/stdint-uintn.h>
#include <filesystem>
#include <ios>
#include <string>
#include <sys/types.h>
#include <fstream>


#include "MinecraftHandler.h"
#include "PlayerHandler.h"
#include "SFW/Serializer.h"
#include "nlohmann/json.hpp"
#include "DataTypes/Identifier.h"
#include "DataTypes/nbt.h"
#include "utils.h"
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

            for (int x = 0; x < 32; ++x)
            {
                for (int y = 0; y < 32; ++y)
                {
                    //This can be done better
                    //Instead of recomputing the offset of the chunk everytime
                    //Just go forwards untill next 4096 multiple
                    //The chunks are 4096 bytes padded
                    //MAYBE
                    
                    size_t chunkLocationOffset = 4 * ((x & 31) + (y & 31) * 32);
                    data.seekg(chunkLocationOffset);
                    
                    char chunkLocation[4] = {0,0,0,0};
                    int chunkOffset;
                    data.read((char*)&chunkOffset, 3);
                    data.read(chunkLocation, 1);
                    chunkOffset = (util::byteswap(chunkOffset) >> 8) * 4096 ;
                    [[maybe_unused]]
                    int sectorCount = chunkLocation[0];
                    data.seekg(chunkOffset, data.beg);
                    
                    char lengthAndCompression[5];
                    data.read(lengthAndCompression, 5);
                    
                    zstr::istreambuf decompressionBuffer(data.rdbuf());
                    std::istream decompressedStream(&decompressionBuffer);
                    
                    SFW_LOG_INFO("loadChunkRegion", "x{}, y{}", x, y);
                    out[x][y] = NBT::parse(decompressedStream);
                }
            }

            return out;
        }
    }

    MinecraftHanlder::MinecraftHanlder()
        : m_stop(false)
    {
        BuildRegistryPackets();
        //m_context.chunk_region = loadChunkRegion("r.0.0.mca");
    }

    void MinecraftHanlder::OnConnected(iu::Connection& connection)
    {
        SFW_LOG_INFO("Handler", "New connection from: {}:{}", connection.GetAdress(), connection.GetPort());
    }

    void MinecraftHanlder::HandleConnection(iu::Connection &connection)
    {
        PlayerHandler h(connection, m_context);
        std::vector<uint8_t> data;
        data.resize(PACKET_SIZE);
        std::stringstream ss;
        while(!m_stop)
        {
            size_t recv = connection.Receive(data);
            for(size_t i = 0; i < recv; ++i)
            {
                ss << std::hex << (int)data[i] << ", ";
            }

            SFW_LOG_DEBUG("MinecraftHandler", "{}", ss.str());
            ss.clear();
            ss.str("");
            if (recv == 0 )
                return;
            h.Execute(data);
        }
    }

    void MinecraftHanlder::Stop()
    {
        return;
    }

    //Private

    //These are semi hardcoded and inflexible for now in the name of progress
    void MinecraftHanlder::BuildRegistryPackets()
    {
        SFW_LOG_INFO("MinecraftHandler", "Building registry packs");
        for (const auto& [registry, packetFile] : std::ranges::views::zip(m_context.registry_packets, std::filesystem::directory_iterator("packets")))
        {
            SFW_LOG_DEBUG("MinecraftHandler", "Loading packet {}", packetFile.path().filename().string());
            std::ifstream packet(packetFile.path(), std::ios::binary);
            const size_t fileSize = packetFile.file_size();
            registry.resize(fileSize);
            packet.read(reinterpret_cast<char*>(registry.data()), fileSize);
        }
    }

}