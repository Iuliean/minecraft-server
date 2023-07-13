#ifndef UTILS_H
#define UTILS_H

#include <bits/iterator_concepts.h>
#include <bits/stdint-uintn.h>
#include <ios>
#include <spdlog/fmt/fmt.h>
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <sstream>
#include <iomanip>
#include <vector>
namespace mc
{
    namespace util
    {
        constexpr inline uint8_t SEGMENT_BIT    = 0x7F;
        constexpr inline uint8_t CONTINUE_BIT   = 0x80;

        template<typename T>
        concept IteratorU8 = 
          std::same_as<typename T::value_type, uint8_t> &&
          std::input_or_output_iterator<T>;

        class uuid
        {
        public:
            uuid(auto&& data)
            {
                static_assert(util::IteratorU8<std::remove_reference_t<decltype(data)>>);
                for(uint8_t i = 0; i < 16; i++)
                    m_data[i] = *data++;
            }

            uint8_t* begin()
            {
                return m_data;
            }

            uint8_t* end()
            {
                return m_data + 16;
            }

            const uint8_t* begin()const
            {
                return m_data;
            }

            const uint8_t* end()const
            {
                return m_data + 16;
            }

            std::string AsString()const
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
        private:
            uint8_t m_data[16];
        };

        int readVarInt(auto&& begin)
        {
            static_assert(IteratorU8<std::remove_reference_t<decltype(begin)>>);
            int value = 0;
            uint8_t position = 0;

            while(true)
            {
                value |= (*begin & SEGMENT_BIT) << position;

                if((*begin & CONTINUE_BIT) == 0)
                {
                    ++begin;
                    return value;
                }
                
                position += 7;

                if(position >= 32)
                    throw std::runtime_error("VarInt too big");
                
                ++begin;
            }
            return value;
        }

        //MAKE UTF-8 FOR THE LOVE OF GOD PLZZ
        //currently does not spport UTF-8 becareful when using
        std::string readString(auto&& begin)
        {
            static_assert(IteratorU8<std::remove_reference_t<decltype(begin)>>);
            const int strSize = readVarInt(begin);
            std::string out;
            //doesnt work with utf-8 actual size might need to be bigger;   
            out.append(begin, begin + strSize);
            begin += strSize;
            return out;
        }
        
        void writeVarInt(std::vector<uint8_t>& buffer, int value);
        void writeVarInt(std::vector<uint8_t>&buffer, size_t pos, int value);
        void writeStringToBuff(std::vector<uint8_t>& buffer, std::string_view str);
    }

}

//FMT FORMATTERS

template<>
struct fmt::formatter<mc::util::uuid> : fmt::formatter<std::string>
{
    auto format(const mc::util::uuid& my, format_context &ctx) const -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "{}", my.AsString());
    }
};

#endif //UTILS_H