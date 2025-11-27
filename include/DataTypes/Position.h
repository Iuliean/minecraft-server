#ifndef POSITION_H
#define POSITION_H

#include <cstdint>

#include "utils.hpp"
namespace mc
{
    class position
    {
    public:
        constexpr position(std::int32_t x, std::int32_t z, std::int16_t y)
        {
            assign(x, z, y);
        }

        constexpr position(position& other)  = default;
        constexpr position(position&& other) = default;

        constexpr position& operator=(position& other) = default;
        constexpr position& operator=(position&& other) = default;

        // All gets here have to use shift operators.
        // https://wiki.vg/Protocol#Position
        constexpr std::int32_t get_x() const noexcept
        {
            return m_positions >> 38;
        }

        constexpr std::int32_t get_z() const noexcept
        {
            return m_positions << 26 >> 38;
        }

        constexpr std::int16_t get_y() const noexcept
        {
            return m_positions << 52 >> 52;
        }

        constexpr std::int64_t get() const noexcept
        {
            return m_positions;
        }

        constexpr position& set_x(std::int32_t x) noexcept
        {
            assign(x, get_z(), get_y());
            return *this;
        }

        constexpr position& set_z(std::int32_t z) noexcept
        {
            assign(get_x(), z, get_y());
            return *this;
        }

        constexpr position& set_y(std::int16_t y) noexcept
        {
            assign(get_x(), get_z(), y);
            return *this;
        }

        constexpr inline void assign(std::int32_t x, std::int32_t z, std::int16_t y)
        {
            ASSERT(x <= 0x3FFFFFF, "X too big");
            ASSERT(z <= 0x3FFFFFF, "z too big");
            ASSERT(y <= 0xFFF, "y too big");

            m_positions = ((static_cast<std::int64_t>(x) & XBITS) << 38) |
                          ((static_cast<std::int64_t>(z) & ZBITS) << 12) |
                          (y & YBITS);
        }

    private:
        friend std::formatter<mc::position>;
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
struct std::formatter<mc::position> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::position& obj, format_context &ctx) const
    {
        return std::format_to(ctx.out(), "{:0b}", obj.get());
    }
};

template<>
struct iu::Serializer<mc::position>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::position& toSerialize)
    {
        mc::util::LongSerializer serializer;
        serializer.Serialize(buffer, toSerialize.get());
    }
};
#endif
