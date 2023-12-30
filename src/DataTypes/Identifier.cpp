#include "DataTypes/Identifier.h"

namespace mc
{
        std::regex Identifier::s_allowedCharacters{"[a-z0-9.-_]"};

        Identifier::Identifier(const std::string& category, const std::string& value)
        {
            m_value = value;
            m_category = category;

            util::toLower(m_value);
            util::toLower(m_category);

            if (!std::regex_match(m_value, s_allowedCharacters))
                throw std::runtime_error("Value does not respect character restrictions");
            if (!std::regex_match(m_category, s_allowedCharacters))
                throw std::runtime_error("Category does not respect character restrictions");
        }

        Identifier::Identifier(const std::string& value)
            :m_category("minecraft")
        {
            m_value = value;
            util::toLower(m_value);
            
            if (!std::regex_match(m_value, s_allowedCharacters))
                throw std::runtime_error("Value does not respect character restrictions");
        }
}