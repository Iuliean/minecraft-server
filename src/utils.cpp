#include "utils.h"

namespace mc
{
    namespace util
    {
        
        uuid::uuid()
        {
            memset(m_data, 0, 16);
        }
        std::string uuid::AsString()const
        {
            std::stringstream ss;
            ss << std::hex;
            ss << (int)m_data[0]
            << (int)m_data[1]
            << (int)m_data[2]
            << (int)m_data[3]
            << (int)m_data[4]
            << (int)m_data[5]
            << (int)m_data[6]
            << (int)m_data[7]
            << (int)m_data[8]
            << (int)m_data[9]
            << (int)m_data[10]
            << (int)m_data[11]
            << (int)m_data[12]
            << (int)m_data[13]
            << (int)m_data[14]
            << (int)m_data[15];
            return ss.str();
        }

        void uuid::Serialize(std::vector<uint8_t> &buffer)const
        {
            buffer.insert(buffer.end(), m_data, m_data + 16);
        }

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