#ifndef UTILS_H
#define UTILS_H

#include <SFW/Serializer.h>
#include <SFW/LoggerManager.h>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <nlohmann/json.hpp>

#include <bits/iterator_concepts.h>
#include <bits/stdint-uintn.h>
#include <spdlog/fmt/fmt.h>
#include <stdexcept>
#include <ranges>
#include <map>
#include <string_view>
#include <iterator>
#include <type_traits>
#include <vector>
#include <variant>
#include <format>

namespace mc
{

    namespace server
    {
        enum class IdlePacketID;
        enum class StatusPacketID;
        enum class ConfigPacketID;
        enum class LoginPacketID;
    } // namespace server

    namespace client
    {
        enum class IdlePacketID;
        enum class StatusPacketID;
        enum class LoginPacketID;
        enum class ConfigPacketID;
        enum class PlayPacketID;
    } // namespace client

    namespace util
    {
        constexpr inline uint8_t SEGMENT_BIT  = 0x7F;
        constexpr inline uint8_t CONTINUE_BIT = 0x80;

        template<typename T>
        concept IteratorU8 = 
            std::same_as<typename T::value_type, uint8_t> && std::input_or_output_iterator<T>;

        template<typename T>
        concept PacketID =
            std::same_as<T, int> || std::same_as<T, client::IdlePacketID> ||
            std::same_as<T, server::IdlePacketID> || std::same_as<T, client::StatusPacketID>   ||
            std::same_as<T, server::StatusPacketID> || std::same_as<T, client::LoginPacketID>  ||
            std::same_as<T, server::ConfigPacketID> || std::same_as<T, client::ConfigPacketID> ||
            std::same_as<T, server::LoginPacketID> || std::same_as<T, client::PlayPacketID>;

        template<typename T>
        concept Numeric = std::integral<T> || std::floating_point<T>;
        class uuid
        {
        public:
            uuid();

            template<IteratorU8 Iter>
            uuid(Iter& data)
            {
                for(uint8_t i = 0; i < 16; i++)
                    m_data[i] = *data++;
            }

            inline uint8_t* begin() { return m_data; }

            inline uint8_t* end() { return m_data + 16; }

            inline const uint8_t* begin() const { return m_data; }

            inline const uint8_t* end() const { return m_data + 16; }

            std::string AsString() const;
            void Serialize(std::vector<uint8_t>& buffer) const;

        private:
            uint8_t m_data[16];
        };

        inline constexpr int sizeOfVarInt(const int i) { return (i < 127 ? 1 : (i / 127)); }

        inline constexpr int sizeOfString(std::string_view s)
        {
            return sizeOfVarInt(s.size()) + s.size();
        }

        template<IteratorU8 Iter>
        int readVarInt(Iter& begin)
        {
            int value        = 0;
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

        // MAKE UTF-8 FOR THE LOVE OF GOD PLZZ
        // currently does not spport UTF-8 becareful when using
        template<IteratorU8 Iter>
        std::string readString(Iter& begin)
        {
            const int strSize = readVarInt(begin);
            std::string out;
            // doesnt work with utf-8 actual size might need to be bigger;
            out.append(begin, begin + strSize);
            begin += strSize;
            return out;
        }

        void writeVarInt(std::vector<uint8_t>& buffer, int value);
        void writeVarInt(std::vector<uint8_t>& buffer, size_t pos, int value);
        void writeStringToBuff(std::vector<uint8_t>& buffer, std::string_view str);

        void toLower(std::string& s);

        
        template<std::integral T>
        constexpr T byteswap(T value) noexcept
        {
            static_assert(std::has_unique_object_representations_v<T>, 
                          "T may not have padding bits");
            auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
            std::ranges::reverse(value_representation);
            return std::bit_cast<T>(value_representation);
        }
    } // namespace util

} // namespace mc

// FMT FORMATTERS
/*
template<typename T> requires (std::ranges::range<T>)
struct std::formatter<T> : std::formatter<std::string>
{
    template<typename FmtContext>
    auto format(const T& my, FmtContext& ctx) const
    {
        for (const auto& v : my)
        {
            std::format_to(ctx.out(), "{}", v);
        }
        return ctx.out();
    }
};*/

template<>
struct std::formatter<mc::util::uuid> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::util::uuid& my, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}", my.AsString());
    }
};

template<>
struct std::formatter<nlohmann::json> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const nlohmann::json& my, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}", my.dump());
    }
};

template<>
struct std::formatter<std::byte> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const std::byte& byte, FmtContext& ctx) const
    {

        return std::format_to(ctx.out(), "b{:#x}", std::to_integer<int>(byte));
    }
};

template<typename ...Args>
struct std::formatter<std::variant<Args...>> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const std::variant<Args...>& my, FmtContext& ctx) const
    {
        std::visit([&ctx](auto&& arg) 
        { std::format_to(ctx.out(), "{}", arg); }
        , my);

        return ctx.out();
    }
};


// SERIALIZERS

template<mc::util::Numeric T>
struct iu::Serializer<T>
{
    void Serialize(std::vector<uint8_t>& buffer, T toSerialize)
    {
        uint8_t* data = reinterpret_cast<uint8_t*>(&toSerialize);
        constexpr size_t container_end = -1;
        for(size_t i = sizeof(toSerialize) - 1; i != container_end; --i)
        {
            buffer.push_back(data[i]);
        }
    }
};

template<std::ranges::input_range R>
struct iu::Serializer<R>
{
    void Serialzie(std::vector<uint8_t>& buffer, const R& range)
    {
        for (const auto& value : range)
        {
            iu::Serializer<decltype(value)>().Serialize(buffer, value);
        }
    }
};

template<>
struct iu::Serializer<std::byte>
{
    consteval size_t GetSize(std::byte value) const noexcept { return 1; }

    void Serialize(std::vector<uint8_t>& buffer, std::byte value) const noexcept
    {
        buffer.push_back(std::bit_cast<uint8_t>(value));
    }
};

template<>
struct iu::Serializer<mc::util::uuid>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::util::uuid& toSerialize)
    {
        for(uint8_t it : toSerialize)
        {
            buffer.push_back(it);
        }
    }
};

template<typename ...Args>
struct iu::Serializer<std::variant<Args...>>
{
    void Serialize(std::vector<uint8_t>& buffer, const std::variant<Args...>& obj)
    {
        std::visit([&buffer](const auto& value) { iu::Serializer<std::remove_cvref_t<decltype(value)>>().Serialize(buffer, value); }, obj );
    }
};
namespace mc::util
{
    using ByteSerialzier = iu::Serializer<std::byte>;
    using BoolSerializer = iu::Serializer<bool>;
    using CharSerializer = iu::Serializer<char>;
    using ShortSerializer = iu::Serializer<std::uint16_t>;
    using IntSerializer  = iu::Serializer<std::int32_t>;
    using LongSerializer = iu::Serializer<std::int64_t>;
    using FloatSerializer = iu::Serializer<float>;
    using DoubleSerializer = iu::Serializer<double>;

    using varInt = int; //packets hold both ints and varInt this helps to distinguish
    using varLong = long; //same as above
} // namespace mc::util
#endif // UTILS_H