#ifndef UUID_HPP
#define UUID_HPP
#include "utils.hpp"

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

    private:
        friend std::formatter<mc::uuid>;
        friend iu::Serializer<mc::uuid>;
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
#endif //UUID_H
