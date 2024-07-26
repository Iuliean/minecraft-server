#ifndef NBT_H
#define NBT_H

#include <SFW/Serializer.h>
#include <concepts>
#include <cstring>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <sys/types.h>
#include <utils.h>
#include <bits/stdint-uintn.h>
#include <unordered_map>
#include <SFW/utils.h>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <fstream>
namespace mc::NBT
{
    enum class TagType
    {

        UNKNOWN      = -2,
        ROOTCOMPOUND = -1,
        END          = 0,
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

    using Byte   = std::int8_t;
    using Short  = std::int16_t;
    using Int    = std::int32_t;
    using Long   = std::int64_t;
    using Float  = float;
    using Double = double;
    using String = std::string;

    template<typename T>
    using Array     = std::vector<T>;
    using ByteArray = Array<Byte>;
    using IntArray  = Array<Int>;
    using LongArray = Array<Long>;

    // only IEE 754 float values
    static_assert(std::numeric_limits<double>::is_iec559);

    class NBTCompound;
    class NBTList;

    template<typename T>
    consteval inline TagType getTagType()
    {
        if constexpr(std::same_as<T, std::int8_t>)
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
        else if constexpr(std::same_as<T, std::vector<std::int8_t>>)
            return TagType::BYTE_ARRAY;
        else if constexpr(std::same_as<T, std::string>)
            return TagType::STRING;
        else if constexpr(std::same_as<T, std::vector<std::int32_t>>)
            return TagType::INT_ARRAY;
        else if constexpr(std::same_as<T, std::vector<std::int64_t>>)
            return TagType::LONG_ARRAY;
        else if constexpr(std::same_as<T, NBTCompound>)
            return TagType::COMPOUND;
        else if constexpr(std::same_as<T, NBTList>)
            return TagType::LIST;
        else
            return TagType::UNKNOWN;
    }

    template<typename T>
    concept CanConstructNBTTag =
        std::same_as<T, Byte> || std::same_as<T, Short> || std::same_as<T, Int> ||
        std::same_as<T, Long> || std::same_as<T, Float> || std::same_as<T, Double> ||
        std::same_as<T, ByteArray> || std::same_as<T, String> || std::same_as<T, IntArray> ||
        std::same_as<T, LongArray> || std::same_as<T, NBTCompound> ||
        std::same_as<T, NBTList>;

    void serializeString(std::vector<std::uint8_t>& buffer, const std::string& data);

    template<typename T>
    void serializeArray(std::vector<std::uint8_t>& buffer, const std::vector<T>& data)
    {
        util::IntSerializer().Serialize(buffer, data.size());
        iu::Serializer<T> s;
        for (const auto element : data)
            s.Serialize(buffer, element);
    }

    class NBTTag
    {
    public:
        NBTTag();
        NBTTag(TagType type);
        virtual ~NBTTag() = default;

        inline TagType Type() const noexcept { return m_tagType; }

        virtual inline NBTTag* Clone() const = 0;

        virtual void Serialize(std::vector<std::uint8_t>& buffer) const = 0;

        virtual std::string AsString() const = 0;

    private:
        TagType m_tagType;
    };

    template<CanConstructNBTTag T>
    class NBTNamedTag : public NBTTag
    {
    public:
        NBTNamedTag(const std::string& name) : NBTTag(getTagType<T>()), m_payload(), m_name(name) {}

        NBTNamedTag(const std::string& name, const T& object)
            : NBTTag(getTagType<T>()),
              m_payload(object),
              m_name(name)
        {
        }

        NBTNamedTag(const std::string& name, T&& object)
            : NBTTag(getTagType<T>()),
              m_payload(std::move(object)),
              m_name(name)
        {
        }

        inline const std::string& GetName() const noexcept { return m_name; }

        inline T& Get() noexcept { return m_payload; }

        inline const T& Get() const noexcept { return m_payload; }

        inline NBTTag* Clone() const override { return new NBTNamedTag<T>(*this); }

        void Serialize(std::vector<std::uint8_t>& buffer) const override
        {
            util::ByteSerializer().Serialize(buffer, (Byte)Type());
            serializeString(buffer, m_name);
            if constexpr (std::same_as<T, std::string>)
            {
                serializeString(buffer, m_payload);
            }
            else if constexpr (std::same_as<T, ByteArray> || std:: same_as<T, IntArray> || std::same_as<T, LongArray>)
            {
                serializeArray(buffer, m_payload);
            }
            else
            {
                iu::Serializer<T>().Serialize(buffer, m_payload);
            }
        }

        inline std::string AsString () const override
        {
            return std::format("{}:{}", m_name, m_payload);
        }

        inline operator T&() noexcept { return m_payload; }

        inline operator const T&() const noexcept { return m_payload; }

        inline T* operator->() noexcept { return &m_payload; }

    private:
        T m_payload;
        std::string m_name;
    };

    template<CanConstructNBTTag T>
    class NBTUnnamedTag : public NBTTag
    {
    public:
        NBTUnnamedTag(const T& object) : NBTTag(getTagType<T>()), m_payload(object) {}

        NBTUnnamedTag(T&& object) : NBTTag(getTagType<T>()), m_payload(std::move(object)) {}

        inline T& Get() noexcept { return m_payload; }

        inline const T& Get() const noexcept { return m_payload; }

        inline NBTTag* Clone() const override { return new NBTUnnamedTag<T>(*this); }

        void Serialize(std::vector<std::uint8_t>& buffer) const override
        {
            if constexpr (std::same_as<T, std::string>)
            {
                serializeString(buffer, m_payload);
            }
            else if constexpr (std::same_as<T, ByteArray> || std:: same_as<T, IntArray> || std::same_as<T, LongArray>)
            {
                serializeArray(buffer, m_payload);
            }
            else
            {
                iu::Serializer<T>().Serialize(buffer, m_payload);
            }
        }

        inline std::string AsString () const override
        {
            return std::format("{}", m_payload);
        }

        inline operator T&() noexcept { return m_payload; }

        inline operator const T&() const noexcept { return m_payload; }

    private:
        T m_payload;
    };

    class NBTCompound
    {
    public:
        using TagMap        = std::unordered_map<std::string, std::unique_ptr<NBTTag>>;
        NBTCompound() = default;
        NBTCompound(const NBTCompound& other);
        NBTCompound(NBTCompound&&) = default;

        NBTCompound& operator=(const NBTCompound& other);
        NBTCompound& operator=(NBTCompound&&) = default;

        ~NBTCompound() = default;

        template<CanConstructNBTTag T>
        NBTNamedTag<T>& Insert(const NBTNamedTag<T>& object)
        {
            auto obj = new NBTNamedTag<T>(object);
            m_objectsTree.insert(std::make_pair(object.GetName(), obj));
            return *obj;
        }

        template<CanConstructNBTTag T>
        NBTNamedTag<T>& Insert(NBTNamedTag<T>&& object)
        {
            auto obj = new NBTNamedTag<T>(std::move(object));
            m_objectsTree.insert(std::make_pair(obj->GetName(), obj));
            return *obj;
        }

        template<CanConstructNBTTag T>
        NBTNamedTag<T>& Insert(const std::string& name, const T& value)
        {
            auto insertLocation = m_objectsTree.insert(
                std::make_pair(name, std::make_unique<NBTNamedTag<T>>(name, value))
            );
            return *insertLocation.first->second.get();
        }

        template<CanConstructNBTTag T>
        NBTNamedTag<T>& Insert(const std::string& name, T&& value)
        {
            auto insertLocation = m_objectsTree.insert(
                std::make_pair(name, std::make_unique<NBTNamedTag<T>>(name, std::move(value)))
            );
            return *dynamic_cast<NBTNamedTag<T>*>(insertLocation.first->second.get());
        }

        void Remove(const std::string& name)
        {
            auto iter = m_objectsTree.find(name);
            if(iter != m_objectsTree.end())
            {
                m_objectsTree.erase(iter);
            }
        }

        template<CanConstructNBTTag T>
        NBTNamedTag<T>& Get(const std::string& name)
        {
            auto iter = m_objectsTree.find(name);
            if(iter == m_objectsTree.end())
            {
                throw std::out_of_range("Element does not exist");
            }
                ASSERT(getTagType<T>() == iter->second->Type(),
                    "Requested type differs from actual type");
                return *dynamic_cast<NBTNamedTag<T>*>(iter->second.get());
        }

        template<CanConstructNBTTag T>
        const NBTNamedTag<T>& Get(const std::string& name) const
        {
            auto iter = m_objectsTree.find(name);
            if(iter == m_objectsTree.end())
            {
                throw std::out_of_range("Element does not exist");
            }
                ASSERT(getTagType<T>() == iter->second->Type(),
                    "Requested type differs from actual type");
                return *dynamic_cast<NBTNamedTag<T>*>(iter->second.get());
        }

        [[nodiscard("You might want to use the output of Contains :)")]]
        bool Contains (const std::string& name) const noexcept { return m_objectsTree.find(name) != end();}

        TagMap::iterator begin() { return m_objectsTree.begin(); }

        TagMap::iterator end() { return m_objectsTree.end(); }

        TagMap::const_iterator begin() const { return m_objectsTree.begin(); }

        TagMap::const_iterator end() const { return m_objectsTree.end(); }

    private:



        friend struct std::formatter<mc::NBT::NBTCompound>; 
        void Copy(const NBTCompound& other);
        TagMap m_objectsTree;
        // needs iterators
    };

    class NBTList
    {
    public:
        using TagPtr = std::unique_ptr<NBTTag>;

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

            inline bool operator!=(const Iterator& other) noexcept
            {
                return m_it != other.m_it;
            }

            inline bool operator<(const Iterator& other) noexcept
            {
                return m_it < other.m_it;
            }

            inline bool operator>(const Iterator& other) noexcept
            {
                return m_it > other.m_it;
            }

            inline bool operator<=(const Iterator& other) noexcept
            {
                return m_it <= other.m_it;
            }

            inline bool operator>=(const Iterator& other) noexcept
            {
                return m_it >= other.m_it;
            }

            inline NBTTag *operator->() noexcept
            {
                return m_it->get();
            }

            template<CanConstructNBTTag T>
            NBTUnnamedTag<T>& get() const
            {
                ASSERT((*m_it)->Type() == getTagType<T>(),
                    "Requested type differs from actual type");
                return *dynamic_cast<NBTUnnamedTag<T>*>(m_it->get());
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

            inline bool operator!=(const ConstIterator& other) noexcept
            {
                return m_it != other.m_it;
            }

            inline bool operator<(const ConstIterator& other) noexcept
            {
                return m_it < other.m_it;
            }

            inline bool operator>(const ConstIterator& other) noexcept
            {
                return m_it > other.m_it;
            }

            inline bool operator<=(const ConstIterator& other) noexcept
            {
                return m_it <= other.m_it;
            }

            inline bool operator>=(const ConstIterator& other) noexcept
            {
                return m_it >= other.m_it;
            }

            inline const NBTTag *operator->() const noexcept
            {
                return m_it->get();
            }

            template<CanConstructNBTTag T>
            const NBTUnnamedTag<T>& get() const
            {
                ASSERT((*m_it)->Type() == getTagType<T>(),
                    "Requested type differs from actual type");
                return *(*m_it).get();
            }

        private:
            const TagPtr* m_it;
        };

        NBTList();
        NBTList(TagType type);
        NBTList(const NBTList& other);
        NBTList(NBTList&& other) = default;

        NBTList& operator=(const NBTList& other);
        NBTList& operator=(NBTList&& other) = default;

        inline TagType TagsType() const { return m_tagsType; }

        template<CanConstructNBTTag T>
        inline NBTUnnamedTag<T>& Insert(const NBTUnnamedTag<T>& object)
        {
            if(m_tagsType == TagType::UNKNOWN)
                m_tagsType = object.Type();
            // Make sure you are requesting the same type that the object contains
            ASSERT(getTagType<T>() == m_tagsType, "Tags type is different than the inserted type");
            m_objectsList.emplace_back(object.Clone());
            return *dynamic_cast<NBTUnnamedTag<T>*>(m_objectsList.back().get());
        }

        template<CanConstructNBTTag T>
        inline NBTUnnamedTag<T>& Insert(NBTUnnamedTag<T>&& object)
        {
            if(m_tagsType == TagType::UNKNOWN)
                m_tagsType = object.Type();
            // Make sure you are requesting the same type that the object contains
            ASSERT(getTagType<T>() == m_tagsType, "Tags type is different than the inserted type");
            m_objectsList.push_back(std::make_unique<NBTUnnamedTag<T>>(std::move(object)));
            return *dynamic_cast<NBTUnnamedTag<T>*>(m_objectsList.back().get());
        }

        template<CanConstructNBTTag T>
        inline NBTUnnamedTag<T>& Insert(const T& object)
        {
            if(m_tagsType == TagType::UNKNOWN)
                m_tagsType = getTagType<T>();
            // Make sure you are requesting the same type that the object contains
            ASSERT(getTagType<T>() == m_tagsType, "Tags type is different than the inserted type");
            m_objectsList.push_back(std::make_unique<NBTUnnamedTag<T>>(object));
            return *dynamic_cast<NBTUnnamedTag<T>*>(m_objectsList.back().get());
        }

        template<CanConstructNBTTag T>
        inline NBTUnnamedTag<T>& Insert(T&& object)
        {
            if(m_tagsType == TagType::UNKNOWN)
                m_tagsType = getTagType<T>();
            // Make sure you are requesting the same type that the object contains
            ASSERT(getTagType<T>() == m_tagsType, "Tags type is different than the inserted type");
            m_objectsList.push_back(std::make_unique<NBTUnnamedTag<T>>(std::move(object)));
            return *dynamic_cast<NBTUnnamedTag<T>*>(m_objectsList.back().get());
        }

        inline size_t Size() const { return m_objectsList.size(); }

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

        void Assign(const NBTList& other);

        inline Iterator begin() { return &m_objectsList.front(); }

        inline Iterator end() { return &m_objectsList.back() + 1; }

        inline ConstIterator begin() const { return &m_objectsList.front(); }

        inline ConstIterator end() const { return &m_objectsList.back() + 1; }

    private:
        std::vector<TagPtr> m_objectsList;
        TagType m_tagsType;
    };

    using NamedByte      = NBTNamedTag<Byte>;
    using NamedShort     = NBTNamedTag<Short>;
    using NamedInt       = NBTNamedTag<Int>;
    using NamedLong      = NBTNamedTag<Long>;
    using NamedFloat     = NBTNamedTag<Float>;
    using NamedDouble    = NBTNamedTag<Double>;
    using NamedString    = NBTNamedTag<String>;
    using NamedByteArray = NBTNamedTag<ByteArray>;
    using NamedIntArray  = NBTNamedTag<IntArray>;
    using NamedLongArray = NBTNamedTag<LongArray>;
    using NamedCompound  = NBTNamedTag<NBTCompound>;
    using NamedList      = NBTNamedTag<NBTList>;

    using UnnamedByte      = NBTUnnamedTag<Byte>;
    using UnnamedShort     = NBTUnnamedTag<Short>;
    using UnnamedInt       = NBTUnnamedTag<Int>;
    using UnnamedLong      = NBTUnnamedTag<Long>;
    using UnnamedFloat     = NBTUnnamedTag<Float>;
    using UnnamedDouble    = NBTUnnamedTag<Double>;
    using UnnamedString    = NBTUnnamedTag<String>;
    using UnnamedByteArray = NBTUnnamedTag<ByteArray>;
    using UnnamedIntArray  = NBTUnnamedTag<IntArray>;
    using UnnamedLongArray = NBTUnnamedTag<LongArray>;
    using UnnamedCompound  = NBTUnnamedTag<NBTCompound>;

    using NBT = NamedCompound;
    using NBTSerializer = iu::Serializer<NBT>;

    NBT parse(std::istream& data);

    inline NBT parse(std::filesystem::path file)
    {
        std::ifstream f(file.c_str(), std::ios::binary);
        return parse(f);
    }

} // namespace mc::NBT

//FMT FORMATTERS

template<>
struct std::formatter<mc::NBT::NBTTag> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::NBT::NBTTag& my, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}", my.AsString());
    }
};

template<mc::NBT::CanConstructNBTTag T>
struct std::formatter<mc::NBT::NBTNamedTag<T>> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::NBT::NBTNamedTag<T>& my, FmtContext& ctx) const
    {
        if(std::same_as<decltype(my.Get()), std::vector<int>>)
            return ctx.out();
        return std::format_to(ctx.out(), "\"{}\":{{ {} }}", my.GetName(), my.Get());
    }
};

template<mc::NBT::CanConstructNBTTag T>
struct std::formatter<mc::NBT::NBTUnnamedTag<T>> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::NBT::NBTUnnamedTag<T>& my, FmtContext& ctx) const
    {
        if(std::same_as<decltype(my.Get()), std::vector<int>>)
            return ctx.out();
        return std::format_to(ctx.out(), "{}", my.Get());
    }
};

template<>
struct std::formatter<mc::NBT::NBTList> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::NBT::NBTList& my, FmtContext& ctx) const
    {
        for(auto it = my.begin(); it < my.end(); it++)
        {
            std::format_to(ctx.out(), "{}", it->AsString());
        }
        return ctx.out();
    }
};

template<>
struct std::formatter<mc::NBT::NBTCompound> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::NBT::NBTCompound& my, FmtContext& ctx) const
    {
        for(auto& it : my)
        {
            std::format_to(ctx.out(), "{}", it.second->AsString());
        }

        return ctx.out();
    }
};

//SERIALIZERS
    template<>
    struct iu::Serializer<mc::NBT::NBTCompound>
    {
        void Serialize(std::vector<uint8_t>& buffer, const mc::NBT::NBTCompound& object)
        {
            for (const auto& [name, tag] : object)
                tag->Serialize(buffer);

            buffer.push_back((uint8_t)mc::NBT::TagType::END);
        }
    };

    template<>
    struct iu::Serializer<mc::NBT::NBTList>
    {
        void Serialize(std::vector<uint8_t>& buffer, const mc::NBT::NBTList& object)
        {
            mc::util::ByteSerializer().Serialize(buffer, (mc::NBT::Byte)object.TagsType());
            mc::util::IntSerializer().Serialize(buffer, object.Size());
            for (auto tag = object.begin(); tag != object.end(); ++tag)
                tag->Serialize(buffer);
        }
    };

    template<mc::NBT::CanConstructNBTTag T>
    struct iu::Serializer<mc::NBT::NBTUnnamedTag<T>>
    {
        void Serialize(std::vector<uint8_t>& buffer, const mc::NBT::NBTUnnamedTag<T>& object)
        {
            object.Serialize(buffer);
        }
    };

    template<mc::NBT::CanConstructNBTTag T>
    struct iu::Serializer<mc::NBT::NBTNamedTag<T>>
    {
        void Serialize(std::vector<uint8_t>& buffer, const mc::NBT::NBTNamedTag<T>& object)
        {
            object.Serialize(buffer);
        }
    };

#endif // NBT_H