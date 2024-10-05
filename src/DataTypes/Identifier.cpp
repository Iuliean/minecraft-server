#include "DataTypes/Identifier.h"

namespace mc
{
        std::regex Identifier::s_allowedNamespaceCharacters{"[a-z0-9.-_]"};
        std::regex Identifier::s_allowedValueCharacters{"[a-z0-9.-_/]"};

        Identifier::Identifier(const std::string& category, const std::string& value)
        {
            m_value = value;
            m_category = category;

            util::toLower(m_value);
            util::toLower(m_category);
            //This regex shit needs to be redone it does not work :)
            if (!std::regex_match(m_value, s_allowedValueCharacters))
                throw std::runtime_error("Value does not respect character restrictions");
            if (!std::regex_match(m_category, s_allowedNamespaceCharacters))
                throw std::runtime_error("Category does not respect character restrictions");
        }

        Identifier::Identifier(const std::string& value)
            :m_category("minecraft")
        {
            m_value = value;
            util::toLower(m_value);

            if (std::regex_match(m_value, s_allowedValueCharacters))
                throw std::runtime_error("Value does not respect character restrictions");
        }
}