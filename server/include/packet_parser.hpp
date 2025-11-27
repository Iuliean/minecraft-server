#ifndef PACKET_PARSER_HPP
#define PACKET_PARSER_HPP
#include "client_packets.hpp"
#include "packet.h"
#include <cstdint>

namespace mc
{
    enum class state
    {
        IDLE,
        STATUS,
        LOGIN,
        CONFIG,
        PLAY
    };

    class packet_parser
    {
    public:
        virtual ~packet_parser() = default;

        virtual void set_state(state new_state) = 0;
        virtual packet_ptr parse(std::span<const uint8_t> buffer) = 0;

    };
}

#endif //PACKET_PARSER_HPP