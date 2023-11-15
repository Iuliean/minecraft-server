#ifndef PACKET_H
#define PACKET_H

#include <spdlog/fmt/fmt.h>
#include <memory>
#include <type_traits>

#include "utils.h"

namespace mc
{

     class Packet
        {
        public:
            using PacketPtr = std::unique_ptr<Packet>;

            template<util::PacketID T>
            Packet(T id)
                :m_id((int)(id))
            {
            }

            virtual ~Packet() = default;

            template<util::PacketID T>
            typename std::remove_const<T>::type GetId()const;
            
            virtual std::string AsString()const; 
            virtual constexpr const char* PacketName()const;
        protected:
            int m_id;
        };

        template<util::PacketID T>
        inline typename std::remove_const<T>::type Packet::GetId()const
        {
            return (typename std::remove_const<T>::type)m_id;
        }
        
        inline std::string Packet::AsString()const
        {
            return "{Id: " + std::to_string(m_id) + "}";
        } 

        inline constexpr const char* Packet::PacketName()const
        {
            return "Packet";
        }

}

//FMT FORMATTERS

template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<mc::Packet, T>::value, char>> :
    fmt::formatter<std::string> {
  auto format(const mc::Packet& a, format_context& ctx) const {
    return fmt::formatter<std::string>::format(fmt::format("{}:{}", a.PacketName(), a.AsString()), ctx);
  }
};

#endif //PACKET_H