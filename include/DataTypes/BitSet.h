#ifndef BIT_SET_H
#define BIT_SET_H
#include "utils.h"
#include <cstdint>
#include <vector>

namespace mc
{
    class BitSet
    {
    public:
        BitSet() = default;
        inline BitSet(int size) : m_data((size / (sizeof(long) * 8 )) + 1, 0){}

        inline bool Test(int index)const
        {
            return (m_data[index / 64] & (1 << ( index % 64))) != 0;
        }
        inline void Set(int index, bool value)
        {
            m_data[index / 64] = m_data[index / 64] | (1 << (index % 64)); 
        }
    private:
        friend struct iu::Serializer<mc::BitSet>;
        std::vector<long> m_data;
    };

    using BitSetSerializer = struct iu::Serializer<mc::BitSet>;
}

template<>
struct iu::Serializer<mc::BitSet>
{
    inline void Serialize(std::vector<uint8_t>& buffer, const mc::BitSet& object)
    {
        mc::util::writeVarInt(buffer, object.m_data.size());
        iu::Serializer<decltype(object.m_data)>().Serialize(buffer, object.m_data);
    }
};

#endif