#include <SFW/LoggerManager.h>
#include <SFW/Connection.h>
#include <algorithm>
#include <bit>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <filesystem>
#include <ios>
#include <ranges>
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

                NBT::NBT chunk = NBT::parse(stream);

                const size_t x = chunk->Get<NBT::Int>("xPos");
                const size_t z = chunk->Get<NBT::Int>("zPos");
                SFW_LOG_INFO("chunkLoading","Chunk x{}, z{}", x, z);

                out[x][z].emplace(std::move(chunk));
            }
            return out;
        }
    }

    MinecraftHanlder::MinecraftHanlder()
        : m_stop(false)
    {
        BuildRegistryPackets();
        m_context.chunk_region = loadChunkRegion("map/r.0.0.mca");
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