#ifndef POSITION_H
#define POSITION_H

#include <cstdint>

#include "utils.h"
namespace mc
{
    class Position
    {
    public:
        constexpr Position(std::int32_t x, std::int32_t z, std::int16_t y)
        {
            Assign(x, z, y);
        }

        Position(Position& other)  = default;
        Position(Position&& other) = default;

        Position& operator=(Position& other) = default;
        Position& operator=(Position&& other) = default;

        // All gets here have to use shift operators.
        // https://wiki.vg/Protocol#Position
        constexpr inline std::int32_t GetX() const noexcept
        {
            return m_positions >> 38;
        }

        constexpr inline std::int32_t GetZ() const noexcept
        {
            return m_positions << 26 >> 38;
        }

        constexpr inline std::int16_t GetY() const noexcept
        {
            return m_positions << 52 >> 52;
        }

        constexpr inline std::int64_t Get() const noexcept
        {
            return m_positions;
        }

        constexpr inline Position& SetX(std::int32_t x) noexcept
        {
            Assign(x, GetZ(), GetY());
            return *this;
        }

        constexpr inline Position& SetZ(std::int32_t z) noexcept
        {
            Assign(GetX(), z, GetY());
            return *this;
        }

        constexpr inline Position& SetY(std::int16_t y) noexcept
        {
            Assign(GetX(), GetZ(), y);
            return *this;
        }

        constexpr inline void Assign(std::int32_t x, std::int32_t z, std::int16_t y)
        {
            ASSERT(x <= 0x3FFFFFF, "X too big");
            ASSERT(z <= 0x3FFFFFF, "z too big");
            ASSERT(y <= 0xFFF, "y too big");

            m_positions = ((static_cast<std::int64_t>(x) & XBITS) << 38) |
                          ((static_cast<std::int64_t>(z) & ZBITS) << 12) |
                          (y & YBITS);
        }

        inline std::string AsString() const
        {
            return std::format("X:{}, Z:{}, Y:{}", GetX(), GetZ(), GetY());
        } 

    private:
        // Top 26 bits of X
        constexpr static inline std::int32_t XBITS = 0x3FFFFFF;
        // Middle 26 bits of z
        constexpr static inline std::int32_t ZBITS = 0x3FFFFFF;
        // Last 12 bits of y
        constexpr static inline std::int16_t YBITS = 0xFFF;

        std::int64_t m_positions;
    };
} // namespace mc


template<>
struct std::formatter<mc::Position> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::Position& my, format_context &ctx) const
    {
        return std::format_to(ctx.out(), "{}", my.AsString());
    }
};

template<>
struct iu::Serializer<mc::Position>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::Position& toSerialize)
    {   
        mc::util::LongSerializer serializer;
        serializer.Serialize(buffer, toSerialize.Get());
    }
};
#endif