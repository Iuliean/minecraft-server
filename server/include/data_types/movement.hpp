#ifndef MOVEMENT_HPP
#define MOVEMENT_HPP
#include <string_view>
#include <utility>

namespace mc
{
    enum class input
    {
        forward = 0x1,
        backward = 0x2,
        left = 0x4,
        right = 0x8,
        jump = 0x10,
        sneak = 0x20,
        sprint = 0x40
    };

    constexpr std::string_view input_as_string(input i)
    {
        switch(i)
        {
            using enum input;
            case forward: return "forward";
            case backward: return "backward";
            case left: return "left";
            case right: return "right";
            case jump: return "jump";
            case sneak: return "sneak";
            case sprint: return "sprint";
        }
        std::unreachable();
    }
}

#endif //MOVEMENT_HPP