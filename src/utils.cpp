#include "utils.h"

namespace mc
{
    namespace util
    {
        void writeVarInt(std::vector<uint8_t>& buffer, int value)
        {
            while (true)
            {
                if ((value & (~SEGMENT_BIT)) == 0)
                {
                    buffer.push_back(value);
                    return;
                }
            buffer.push_back((value & SEGMENT_BIT) | CONTINUE_BIT);
            value >>= 7;
            }
        
        }

        void writeVarInt(std::vector<uint8_t>&buffer, size_t pos, int value)
        {
            while (true)
            {
                if ((value & (~SEGMENT_BIT)) == 0)
                {
                    buffer.insert(buffer.begin()+pos,value);
                    return;
                }
            buffer.insert(buffer.begin() + pos, (value & SEGMENT_BIT) | CONTINUE_BIT);
            value >>= 7;
            ++pos;
            }
        
        }
    
        void writeStringToBuff(std::vector<uint8_t>& buffer, std::string_view str)
        {
            writeVarInt(buffer, str.size());

            for(const char c : str)
            {
                buffer.push_back(c);
            }
        }
    
    }
}