#ifndef UUID_HPP
#define UUID_HPP
#include "utils.hpp"
#include <atomic>
#include <bit>
#include <compare>
#include <cstdint>
#include <cstring>

namespace mc
{
    class uuid
    {
    public:
        constexpr uuid() noexcept
            : m_data(0) {}

        template<util::IteratorU8 Iter>
        constexpr uuid(Iter& data) noexcept
        {
            for(uint8_t i = 0; i < 16; i++)
                m_data[i] = *data++;
        }

        constexpr uint8_t* begin() noexcept { return m_data; }
        constexpr uint8_t* end() noexcept{ return m_data + 16; }
        constexpr const uint8_t* begin() const noexcept { return m_data; }
        constexpr const uint8_t* end() const noexcept { return m_data + 16; }

        constexpr std::strong_ordering operator<=>(const uuid& other) const
        {
            //kinda dumb
            //could've compared the upper and lower halfs but whatever
            for(std::size_t idx = 0; idx < 16; ++idx)
            {
                const std::strong_ordering order = m_data[15 - idx] <=> other.m_data[15 - idx];
                if (order == std::strong_ordering::less || order == std::strong_ordering::greater)
                    return order;
            }

            return std::strong_ordering::equal;
        }

        constexpr bool operator==(const uuid& other)const = default; 

    private:
        friend std::formatter<mc::uuid>;
        friend iu::Serializer<mc::uuid>;
        friend std::less<mc::uuid>;
        friend std::hash<mc::uuid>;
        uint8_t m_data[16];
    };

}

template<>
struct std::formatter<mc::uuid> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::uuid& uuid, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}", uuid.m_data);
    }
};

template<>
struct iu::Serializer<mc::uuid>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::uuid& uuid)
    {
        for(uint8_t it : uuid)
        {
            buffer.push_back(it);
        }
    }
};

template<>
struct std::less<mc::uuid>
{
    constexpr bool operator()(const mc::uuid& lhs, const mc::uuid& rhs) const
    {
        return lhs < rhs;
    }
};

template<>
struct std::hash<mc::uuid>
{
    static constexpr std::size_t operator()(const mc::uuid& uuid)
    {
        std::uint64_t value = 0;
        std::memcpy(&value, uuid.m_data, sizeof(std::uint64_t));

        return std::hash<std::uint64_t>()(value);
    }
};

#endif //UUID_H
