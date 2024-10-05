#ifndef BLOCK_STATE_H
#define BLOCK_STATE_H

#include "DataTypes/Identifier.h"
#include "SFW/LoggerManager.h"
#include <cstdint>
#include <initializer_list>
#include <string>
#include <utility>
#include <variant>
#include <map>
#include <format>
namespace mc
{
    class BlockState
    {
    public:
        using PropertyValue = std::variant<int, bool, std::string>;
        using PropertyMap = std::map<std::string, PropertyValue>;
        using Property = PropertyMap::value_type;
        
        BlockState()=default;
        BlockState(Identifier blockIdentifier)
            : m_ID(std::move(blockIdentifier))
        {}

        BlockState(Identifier blockIdentifier, std::initializer_list<Property> properties)
            : m_ID(std::move(blockIdentifier)), m_properties(properties)
        {}
        ~BlockState() = default;

        inline const Identifier& GetID() const noexcept { return m_ID; }
        inline const PropertyMap& GetProperties() const noexcept { return m_properties; }

        inline void AddProperty(std::string_view name, PropertyValue value) { m_properties.emplace(name, value); }

    private:
        Identifier m_ID;
        PropertyMap m_properties;
    };
}


namespace std
{

    template<>
    class hash<mc::BlockState>
    {
    public:
        std::uint64_t operator()(const mc::BlockState& blockState) const
        {
#if NDEBUG
            return std::hash<std::string>()
            (
                format("{}[{}]", blockState.GetID(), blockState.GetProperties())
            );
#else
            std::string hashedString = std::format("{}[{}]",blockState.GetID(), blockState.GetProperties());
            std::uint64_t hash = std::hash<std::string>()(hashedString);
            iu::LoggerManager::GlobalLogger().debug("{} -> {}", hashedString, hash);
            return hash; 
#endif
        }
    };
    
    template<>
    struct equal_to<mc::BlockState>
    {
        bool operator()(const mc::BlockState& lhs, const mc::BlockState& rhs) const
        {
            auto& lhsProperties = lhs.GetProperties();
            auto& rhsProperties = rhs.GetProperties();

            if (lhs.GetID() != rhs.GetID())
                return false;

            for (const auto& [property, value] : lhsProperties)
            {
                if(!rhsProperties.contains(property))
                    return false;
                if(value != rhsProperties.at(property))
                    return false;

            }
            return true;
        }
    };
/*
    template<>
    struct less<mc::BlockState>
    {
        constexpr bool operator()(const mc::BlockState& lhs, const mc::BlockState& rhs) const
        {
            const std::string rhss = std::format("{}[{}]", rhs.GetID(), rhs.GetProperties());
            const std::string lhss = std::format("{}[{}]", lhs.GetID(), lhs.GetProperties());

            int result = lhss.compare(rhss);

            return result >= 0 ? false : true;
        }
    };
    */
}

#endif