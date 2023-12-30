#ifndef NBT_H
#define NBT_H
#include "utils.h"
#include <bits/stdint-uintn.h>
#include <cstdint>
#include <string>
#include <map>
#include <type_traits>
#include <memory>
#include <utility>
#include <vector>

namespace mc
{
    namespace NBT
    {
        enum class TagType
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

        using Byte         = std::uint8_t;
        using Short        = std::int16_t;
        using Int          = std::int32_t;
        using Long         = std::int64_t;
        using Float        = float;
        using Double       = double;
        using String       = std::string;
        template<typename T>
        using Array        = std::vector<T>;
        using ByteArray    = Array<Byte>;
        using IntArray     = Array<Int>;
        using LongArray    = Array<Long>;
        
        //only IEE 754 float values  
        static_assert(std::numeric_limits<double>::is_iec559);


        class NBTCompoundObject;
        class NBTListObject;

        template<typename T>
        constexpr static inline TagType getTagType()
        {
            if constexpr(std::same_as<T, std::uint8_t>)
                return TagType::BYTE;
            else if constexpr(std::same_as<T, std::int16_t>)
                return TagType::SHORT;
            else if constexpr(std::same_as<T, std::int32_t>)
                return TagType::INT;
            else if constexpr(std::same_as<T, std::int64_t>)
                return TagType::LONG;
            else if constexpr(std::same_as<T, float>)
                return TagType::FLOAT;
            else if constexpr(std::same_as<T, double>)
                return TagType::DOUBLE;
            else if constexpr(std::same_as<T, std::vector<std::uint8_t>>)
                return TagType::BYTE_ARRAY;
            else if constexpr(std::same_as<T, std::string>)
                return TagType::STRING;
            else if constexpr(std::same_as<T,std::vector<std::int32_t>>)
                return TagType::INT_ARRAY;
            else if constexpr(std::same_as<T, std::vector<std::int64_t>>)
                return TagType::LONG_ARRAY;
            else if constexpr(std::same_as<T, NBTCompoundObject>)
                return TagType::COMPOUND;
            else if constexpr(std::same_as<T, NBTListObject>)
                return TagType::LIST;
            else
                return TagType::UNKNOWN;
        }

        template<typename T>
        concept CanConstructNBTTag = std::same_as<T, Byte> ||
            std::same_as<T, Short> ||
            std::same_as<T, Int> || 
            std::same_as<T, Long> || 
            std::same_as<T, Float>        ||
            std::same_as<T, Double>       ||
            std::same_as<T, ByteArray> ||
            std::same_as<T, String>  ||
            std::same_as<T, IntArray>  ||
            std::same_as<T, LongArray> ||
            std::same_as<T, NBTCompoundObject> ||
            std::same_as<T, NBTListObject>;

        class NBTTag
        {
        public:
            NBTTag(TagType type);
            NBTTag();
            virtual ~NBTTag() = default;

            inline TagType Type()const
            {return m_tagType;}

            virtual inline NBTTag* Clone()const
            {return new NBTTag(*this);}

        private:
            TagType m_tagType;
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

            inline const std::string& GetName()const
            {return m_name;}

            inline T& Get()
            {return m_payload;}

            inline const T& Get() const
            {return m_payload;}

            inline NBTTag* Clone()const override
            {return new NBTNamedTag<T>(*this);}

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

            inline NBTTag* Clone()const override
            {return new NBTUnnamedTag<T>(*this);}

        private:
            T m_payload;
        };
        
        class NBTCompoundObject
        {
        public:
            NBTCompoundObject() = default;
            NBTCompoundObject(const NBTCompoundObject& other);

            ~NBTCompoundObject();

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

            template<CanConstructNBTTag T>
            NBTNamedTag<T>& Get(const std::string& name)
            {
                auto iter = m_objectsTree.find(name);
                if(iter != m_objectsTree.end())
                {
                    ASSERT(getTagType<T>() == iter->second->Type(), "Requested type differs from actual type");
                    return *dynamic_cast<NBTNamedTag<T>*>(iter->second);
                }
                return nullptr;
            }

            template<CanConstructNBTTag T>
            const NBTNamedTag<T>& Get(const std::string& name) const
            {
                auto iter = m_objectsTree.find(name);
                if(iter != m_objectsTree.end())
                {
                    ASSERT(getTagType<T>() == iter->second->Type(), "Requested type differs from actual type");
                    return *dynamic_cast<NBTNamedTag<T>*>(iter->second);
                }
                return nullptr;
            }
        private:
            std::map<std::string, NBTTag*> m_objectsTree;
            //needs iterators
        };
        
        class NBTListObject
        {
        public:
            using TagPtr= std::unique_ptr<NBTTag>;

            class Iterator
            {
                public:
                    Iterator(TagPtr* it);

                    inline Iterator& operator++() noexcept
                    {
                        ++m_it;
                        return *this;
                    }
                    
                    inline Iterator operator++(int) noexcept
                    {
                        Iterator out(m_it);
                        ++m_it;
                        return out;
                    }
                    
                    inline Iterator& operator--() noexcept
                    {
                        --m_it;
                        return *this;
                    }
                    
                    inline Iterator operator--(int) noexcept
                    {
                        Iterator out(m_it);
                        --m_it;
                        return out;
                    }

                    template<CanConstructNBTTag T>
                    NBTUnnamedTag<T>& get() const
                    {
                        ASSERT((*m_it)->Type() == getTagType<T>(), "Requested type differs from actual type");
                        return *(*m_it).get();
                    }
                private:
                TagPtr* m_it;
            };

            class ConstIterator
            {
                public:
                    ConstIterator(const TagPtr* it);
                    
                    inline ConstIterator& operator++() noexcept
                    {
                        ++m_it;
                        return *this;
                    }
                    
                    inline ConstIterator operator++(int) noexcept
                    {
                        ConstIterator out(m_it);
                        ++m_it;
                        return out;
                    }
                    
                    inline ConstIterator& operator--() noexcept
                    {
                        --m_it;
                        return *this;
                    }
                    
                    inline ConstIterator operator--(int) noexcept
                    {
                        ConstIterator out(m_it);
                        --m_it;
                        return out;
                    }
                    
                    template<CanConstructNBTTag T>
                    const NBTUnnamedTag<T>& get() const
                    {
                        ASSERT((*m_it)->Type() == getTagType<T>(), "Requested type differs from actual type");
                        return *(*m_it).get();
                    }
                private:
                const TagPtr* m_it;
            };

            NBTListObject();
            NBTListObject(TagType type);
            NBTListObject(const NBTListObject& other);
            NBTListObject(NBTListObject&& other) = default;

            NBTListObject& operator=(const NBTListObject& other);
            NBTListObject& operator=(NBTListObject&& other) = default;

            inline TagType TagsType()const
            {
                return m_tagsType;
            }

            template<CanConstructNBTTag T>
            inline void Add(const NBTUnnamedTag<T>& object)
            {
                if(m_tagsType == TagType::UNKNOWN)
                    m_tagsType = object.Type();
                //Make sure you are requesting the same type that the object contains
                ASSERT(getTagType<T>() == m_tagsType, "Tags type is different than the inserted type");
                m_objectsList.emplace_back(object.Clone());
            }

            template<CanConstructNBTTag T>
            inline void Add(NBTUnnamedTag<T>&& object)
            {
                if(m_tagsType == TagType::UNKNOWN)
                    m_tagsType = object.Type();
                //Make sure you are requesting the same type that the object contains
                ASSERT(getTagType<T>() == m_tagsType, "Tags type is different than the inserted type");
                m_objectsList.push_back(std::move(object));
            }

            inline size_t Size()const
            {
                return m_objectsList.size();
            }

            template<CanConstructNBTTag T>
            inline NBTUnnamedTag<T>& At(size_t pos)
            {
                ASSERT(pos < Size(), "Out of bounds access");
                return *dynamic_cast<NBTUnnamedTag<T>*>(m_objectsList[pos].get());
            }

            template<CanConstructNBTTag T>
            inline const NBTUnnamedTag<T>& At(size_t pos) const
            {
                ASSERT(pos < Size(), "Out of bounds access");
                return *dynamic_cast<NBTUnnamedTag<T>*>(m_objectsList[pos].get());
            }

            NBTTag& operator[](size_t pos);
            const NBTTag& operator[](size_t pos) const;

            void Assign(const NBTListObject& other);

            inline Iterator Begin()
            {
                return &m_objectsList.front();
            }

            inline Iterator End()
            {
                return &m_objectsList.back() + 1;
            }

            inline ConstIterator Begin()const
            {
                return &m_objectsList.front();
            }

            inline ConstIterator End()const
            {
                return &m_objectsList.back() + 1;
            }
        private:
            std::vector<TagPtr> m_objectsList;
            TagType m_tagsType;
        };

        using NamedByte         = NBTNamedTag<Byte>;
        using NamedShort        = NBTNamedTag<Short>;
        using NamedInt          = NBTNamedTag<Int>;
        using NamedLong         = NBTNamedTag<Long>;
        using NamedFloat        = NBTNamedTag<Float>;
        using NamedDouble       = NBTNamedTag<Double>;
        using NamedString       = NBTNamedTag<String>;
        using NamedByteArray    = NBTNamedTag<ByteArray>;
        using NamedIntArray     = NBTNamedTag<IntArray>;
        using NamedLongArray    = NBTNamedTag<LongArray>;
        using NamedCompound     = NBTNamedTag<NBTCompoundObject>;
        using NamedList         = NBTNamedTag<NBTListObject>;

        using UnnamedByte       = NBTUnnamedTag<Byte>;
        using UnnamedShort      = NBTUnnamedTag<Short>;
        using UnnamedInt        = NBTUnnamedTag<Int>;
        using UnnamedLong       = NBTUnnamedTag<Long>;
        using UnnamedFloat      = NBTUnnamedTag<Float>;
        using UnnamedDouble     = NBTUnnamedTag<Double>;
        using UnnamedString     = NBTUnnamedTag<String>;
        using UnnamedByteArray  = NBTUnnamedTag<ByteArray>;
        using UnnamedIntArray   = NBTUnnamedTag<IntArray>;
        using UnnamedLongArray  = NBTUnnamedTag<LongArray>;
        using UnnamedCompound   = NBTUnnamedTag<NBTCompoundObject>;
        using UnnamedList       = NBTUnnamedTag<NBTListObject>;
    }
}

#endif //NBT_H