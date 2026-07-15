#ifndef BIT_SET_HPP
#define BIT_SET_HPP
#include <cstdint>
#include <vector>

#include "utils.hpp"
namespace mc
{
    class bit_set
    {
    public:
        constexpr bit_set() noexcept = default;
        constexpr bit_set(int size) noexcept : m_data((size / (sizeof(long) * 8 )) + 1, 0){}
        constexpr ~bit_set() = default;

        constexpr bool test(int index)const
        {
            return (m_data[index / 64] & (1 << ( index % 64))) != 0;
        }
        constexpr void set(int index, bool value)
        {
            m_data[index / 64] = m_data[index / 64] | (value << (index % 64));
        }
    private:
        friend struct iu::Serializer<mc::bit_set>;
        std::vector<long> m_data;
    };

    using BitSetSerializer = struct iu::Serializer<mc::bit_set>;
}

template<>
struct iu::Serializer<mc::bit_set>
{
    inline void Serialize(std::vector<uint8_t>& buffer, const mc::bit_set& object)
    {
        mc::util::writeVarInt(buffer, object.m_data.size());
        iu::Serializer<decltype(object.m_data)>().Serialize(buffer, object.m_data);
    }
};

#endif
