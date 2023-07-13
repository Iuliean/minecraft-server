#include <array>
#include <bits/stdint-uintn.h>
#include <iterator>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <Connection.h>
#include <sys/types.h>
#include <utility>


#include "MinecraftHandler.h"
#include "PlayerHandler.h"
#include "client/Packet.h"
#include "spdlog/fmt/bundled/format.h"
#include "utils.h"

namespace mc
{

    namespace 
    {
        constexpr static int PACKET_SIZE = 1024;
    }

    MinecraftHanlder::MinecraftHanlder()
        : m_logger(spdlog::stdout_color_mt("MinecraftHandler"))
    { 
    }

    void MinecraftHanlder::OnConnected(iu::Connection& connection)
    {
        m_logger->info("New connection from: {}:{}", connection.GetAdress(), connection.GetPort());
        PlayerHandler h(connection);
        std::vector<uint8_t> data;
        data.resize(PACKET_SIZE);

        while(true)
        {
            size_t recv = connection.Receive(data, PACKET_SIZE);
            if (recv == 0 )
                return;
            h.Execute(data);
        }
        
    }

    void MinecraftHanlder::HandleConnection(iu::Connection &connection)
    {
        return;
    }

    void MinecraftHanlder::Stop()
    {
        return;
    }

}