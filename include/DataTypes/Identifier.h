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
        Identifier() = default;
        Identifier(const std::string& category, const std::string& value);

        Identifier(const std::string& value);

        inline std::string AsString()const
        {
            return m_category + ':' + m_value;
        }
        

        bool Equal(const Identifier& other)const { return m_category == other.m_category && m_value == other.m_value; }
        bool operator==(const Identifier& other)const { return Equal(other); }
        bool operator!=(const Identifier& other)const { return !Equal(other); }
    private:
        static std::regex s_allowedNamespaceCharacters;
        static std::regex s_allowedValueCharacters;
        std::string m_category;
        std::string m_value;
    };
}

template<>
struct std::formatter<mc::Identifier> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::Identifier& my, FmtContext &ctx) const
    {
        return std::formatter<std::string>::format(my.AsString(), ctx);
    }
};

template<>
struct iu::Serializer<mc::Identifier>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::Identifier& toSerialize)
    {
        const std::string identifier = toSerialize.AsString();
        mc::util::writeStringToBuff(buffer, {identifier.c_str(), identifier.size()});
    }
};
#endif