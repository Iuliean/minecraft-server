#include <array>
#include <bits/stdint-uintn.h>
#include <iterator>
#include <Connection.h>
#include <sys/types.h>
#include <utility>


#include "LoggerManager.h"
#include "MinecraftHandler.h"
#include "PlayerHandler.h"

namespace mc
{

    namespace 
    {
        constexpr static int PACKET_SIZE = 1024;
    }

    MinecraftHanlder::MinecraftHanlder()
        : m_logger(iu::LoggerManager::GetLogger("McHandler")),
        m_stop(false)
    { 
    }

    void MinecraftHanlder::OnConnected(iu::Connection& connection)
    {
        m_logger.info("New connection from: {}:{}", connection.GetAdress(), connection.GetPort());
    }

    void MinecraftHanlder::HandleConnection(iu::Connection &connection)
    {
        PlayerHandler h(connection);
        std::vector<uint8_t> data;
        data.resize(PACKET_SIZE);
        std::stringstream ss;
        while(!m_stop)
        {
            size_t recv = connection.Receive(data, PACKET_SIZE);
            for(size_t i = 0; i < recv; ++i)
            {
                ss << std::hex << (int)data[i] << ", ";
            }

            m_logger.debug("{}",ss.str());
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

}