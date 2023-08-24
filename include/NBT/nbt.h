#ifndef NBT_H
#define NBT_H
#include "utils.h"
#include <bits/stdint-uintn.h>
#include <cstdint>
#include <string>
#include <map>
#include <type_traits>
#include <utility>
#include <vector>
/*##################NOT YET FULLY FUNCTIONAL#################*/
namespace mc
{
    namespace NBT
    {
        enum class NBTTagType
        {
            UNKNOWN = -1,
            END = 0,
            BYTE,
            SHORT,
            INT,
            LONG,
            FLOAT,
            DOUBLE,
            BYTE_ARRAY,
            STRING,
            LIST,
            COMPOUND,
            INT_ARRAY,
            LONG_ARRAY
        };
        
        class NBTCompoundObject;
        class NBTListObject;

        template<typename T>
        constexpr inline NBTTagType getTagType()
        {
            if (std::same_as<T, std::uint8_t>)
                return NBTTagType::BYTE;
            else if (std::same_as<T, std::int16_t>)
                return NBTTagType::SHORT;
            else if(std::same_as<T, std::int32_t>)
                return NBTTagType::INT;
            else if(std::same_as<T, std::int64_t>)
                return NBTTagType::LONG;
            else if(std::same_as<T, float>)
                return NBTTagType::FLOAT;
            else if(std::same_as<T, double>)
                return NBTTagType::DOUBLE;
            else if(std::same_as<T, std::vector<std::uint8_t>>)
                return NBTTagType::BYTE_ARRAY;
            else if(std::same_as<T, std::string>)
                return NBTTagType::STRING;
            else if(std::same_as<T,std::vector<std::int32_t>>)
                return NBTTagType::INT_ARRAY;
            else if(std::same_as<T, std::vector<std::int64_t>>)
                return NBTTagType::LONG_ARRAY;
            else if(std::same_as<T, NBTCompoundObject>)
                return NBTTagType::COMPOUND;
            else if(std::same_as<T, NBTListObject>)
                return NBTTagType::LIST;
            else
                return NBTTagType::UNKNOWN;
        }


        template<typename T>
        concept CanConstructNBTTag = std::same_as<T, std::uint8_t> ||
            std::same_as<T, std::int16_t> ||
            std::same_as<T, std::int32_t> || 
            std::same_as<T, std::int64_t> || 
            std::same_as<T, float>        ||
            std::same_as<T, double>       ||
            std::same_as<T, std::vector<std::uint8_t>> ||
            std::same_as<T, std::string>  ||
            std::same_as<T,std::vector<std::int32_t>>  ||
            std::same_as<T, std::vector<std::int64_t>> ||
            std::same_as<T, NBTCompoundObject> ||
            std::same_as<T, NBTListObject>;

        class NBTTag
        {
        public:
            NBTTag(NBTTagType type);
            NBTTag();
            virtual ~NBTTag() = default;

            NBTTagType Type()const
            {return m_tagType;}
        private:
            NBTTagType m_tagType;
        };

        template<CanConstructNBTTag T>
        class NBTNamedTag : public NBTTag
        {
        public:

            NBTNamedTag(const std::string& name)
                : NBTTag(getTagType<T>()),
                m_payload(),
                m_name(name)
            {}    

            NBTNamedTag(const T& object, const std::string& name)
                :NBTTag(getTagType<T>()),
                m_payload(object),
                m_name(name)
            {}

            NBTNamedTag(T&& object, const std::string& name)
                : NBTTag(getTagType<T>()),
                m_payload(object),
                m_name(name)
            {}
            /*
            NBTNamedTag(const NBTNamedTag& other)
                : NBTTag(other),
                m_payload(other.m_payload),
                m_name(other.m_name)
            {}
            */

            inline const std::string& GetName()const
            {return m_name;}

            inline T& Get()
            {return m_payload;}

            inline const T& Get() const
            {return m_payload;}

        private:
            T m_payload;
            std::string m_name;
        };

        template<CanConstructNBTTag T>
        class NBTUnnamedTag : public NBTTag
        {
        public:
            
            NBTUnnamedTag(const T& object)
                :NBTTag(getTagType<T>()),
                m_payload(object)
            {}

            NBTUnnamedTag(T&& object)
                : NBTTag(getTagType<T>()),
                m_payload(object)
            {}

            inline T& Get()
            {return m_payload;}

            inline const T& Get() const
            {return m_payload;}

        private:
            T m_payload;
        };
        
        using NamedByte         = NBTNamedTag<std::uint8_t>;
        using NamedShort        = NBTNamedTag<std::int16_t>;
        using NamedInt          = NBTNamedTag<std::int32_t>;
        using NamedLong         = NBTNamedTag<std::int64_t>;
        //IEE 754 values
        using NamedFloat        = NBTNamedTag<float>;
        using NamedDouble       = NBTNamedTag<double>;
        using NamedString       = NBTNamedTag<std::string>;
        using NamedByteArray    = NBTNamedTag<std::vector<std::uint8_t>>;
        using NamedIntArray     = NBTNamedTag<std::vector<std::int32_t>>;
        using NamedLongArray    = NBTNamedTag<std::vector<std::int64_t>>;
        using NamedCompound     = NBTNamedTag<NBTCompoundObject>;
        using NamedList         = NBTNamedTag<NBTListObject>;

        using UnnamedByte         = NBTUnnamedTag<std::uint8_t>;
        using UnnamedShort        = NBTUnnamedTag<std::int16_t>;
        using UnnamedInt          = NBTUnnamedTag<std::int32_t>;
        using UnnamedLong         = NBTUnnamedTag<std::int64_t>;
        using UnnamedFloat        = NBTUnnamedTag<float>;
        using UnnamedDouble       = NBTUnnamedTag<double>;
        using UnnamedString       = NBTUnnamedTag<std::string>;
        using UnnamedByteArray    = NBTUnnamedTag<std::vector<std::uint8_t>>;
        using UnnamedIntArray     = NBTUnnamedTag<std::vector<std::int32_t>>;
        using UnnamedLongArray    = NBTUnnamedTag<std::vector<std::int64_t>>;
        using UnnamedCompound     = NBTUnnamedTag<NBTCompoundObject>;
        using UnnamedList         = NBTUnnamedTag<NBTListObject>;

        class NBTCompoundObject
        {
        private:
            std::map<std::string, NBTTag*> m_objectsTree;
            using Iter = decltype(m_objectsTree.begin());
        public:
            NBTCompoundObject() = default;
            /*NBTCompoundObject(NBTCompoundObject&& other)
            {
                for(auto&& tag : m_objectsTree)
                {
                    other.m_objectsTree.insert(tag);
                    m_objectsTree[tag.first] = nullptr;
                }
            }
*/

            ~NBTCompoundObject()
            {
                for(auto tag : m_objectsTree)
                {
                    delete tag.second;
                }
            }

            template<CanConstructNBTTag T>
            void Add(const NBTNamedTag<T>& object)
            {
                auto obj = new NBTNamedTag<T>(object);
                m_objectsTree.insert(std::make_pair(object.GetName(), obj));
            }

            template<CanConstructNBTTag T>
            void Add(NBTNamedTag<T>&& object)
            {
                auto obj = new NBTNamedTag<T>(std::move(object));
                m_objectsTree.insert(std::make_pair(obj->GetName(), obj));
            }

            void Remove(const std::string& name)
            {
                auto iter = m_objectsTree.find(name);
                if(iter != m_objectsTree.end())
                {
                    delete iter->second;
                    m_objectsTree.erase(iter);
                }
            }

            template<typename T>
            NBTNamedTag<T>* Get(const std::string& name)
            {
                auto iter = m_objectsTree.find(name);
                if(iter != m_objectsTree.end())
                {
                    ASSERT(getTagType<T>() == iter->second->Type(), "Requested type differs from actual type");
                    return dynamic_cast<NBTNamedTag<T>*>(iter->second);
                }
                return nullptr;
            }

            template<typename T>
            NBTNamedTag<T>* const Get(const std::string& name) const
            {
                auto iter = m_objectsTree.find(name);
                if(iter != m_objectsTree.end())
                {
                    ASSERT(getTagType<T>() == iter->second->Type(), "Requested type differs from actual type");
                    return dynamic_cast<NBTNamedTag<T>*>(iter->second);
                }
                return nullptr;
            }

            //needs iterators
        };
        
        /* to be implemented
        class NBTListObject
        {
        public:
            NBTListObject() = default;
            NBTListObject(NBTListObject&& other)
            {
                //to implement
            }
        private:
            std::vector<NBTTag*> m_objectsList;
        };
        */
    }
}

#endif //NBT_H