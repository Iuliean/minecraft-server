#ifndef IDENTIFIER_H
#define IDENTIFIER_H
#include <string>
#include <regex>

#include "utils.h"

namespace mc
{
    class Identifier
    {
    public:
        Identifier(const std::string& category, const std::string& value);

        Identifier(const std::string& value);

        inline std::string AsString()const
        {
            return m_category + ':' + m_value;
        }

    private:
        static std::regex s_allowedCharacters;
        std::string m_category;
        std::string m_value;
    };
}

template<>
struct fmt::formatter<mc::Identifier> : fmt::formatter<std::string>
{
    auto format(const mc::Identifier& my, format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", my.AsString());
    }
};

template<>
struct iu::Serializer<mc::Identifier>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::Identifier& toSerialize)
    {   
        mc::util::writeStringToBuff(buffer, toSerialize.AsString());
    }
};
#endif