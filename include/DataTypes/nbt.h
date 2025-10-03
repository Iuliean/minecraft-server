#ifndef NBT_H
#define NBT_H

#include <concepts>
#include <cstddef>
#include <format>
#include <iterator>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <sys/types.h>
#include <type_traits>
#include <bits/stdint-uintn.h>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <fstream>

#include "SFW/Serializer.h"
#include "utils.h"
namespace mc::nbt
{
    enum class tag_type
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

    using Byte   = std::byte;
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

    class compound;
    class list;
    class named_tag;
    class tag;
    using value = std::variant<Byte, Short, Int, Long, Float, Double, String, ByteArray, IntArray, LongArray, compound, list>;
    using nbt = tag;
    class nbt_error : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
        virtual ~nbt_error() = default;
    };

    template<typename T>
    constexpr inline tag_type get_tag_type(const T& v) noexcept
    {
        if constexpr(std::same_as<T, Byte>)
            return tag_type::BYTE;
        else if constexpr(std::same_as<T, Short>)
            return tag_type::SHORT;
        else if constexpr(std::same_as<T, Int>)
            return tag_type::INT;
        else if constexpr(std::same_as<T, Long>)
            return tag_type::LONG;
        else if constexpr(std::same_as<T, Float>)
            return tag_type::FLOAT;
        else if constexpr(std::same_as<T, Double>)
            return tag_type::DOUBLE;
        else if constexpr(std::same_as<T, ByteArray>)
            return tag_type::BYTE_ARRAY;
        else if constexpr(std::same_as<T, String>)
            return tag_type::STRING;
        else if constexpr(std::same_as<T, IntArray>)
            return tag_type::INT_ARRAY;
        else if constexpr(std::same_as<T, LongArray>)
            return tag_type::LONG_ARRAY;
        else if constexpr(std::same_as<T, compound>)
            return tag_type::COMPOUND;
        else if constexpr(std::same_as<T, list>)
            return tag_type::LIST;
        else if (std::same_as<T, tag>)
            return std::visit([](const auto& t) noexcept { return get_tag_type(t);}, v);
        else
            return tag_type::UNKNOWN;
    }
    template<typename T>
    concept nbt_tag =
        std::same_as<T, Byte> || std::same_as<T, Short> || std::same_as<T, Int> ||
        std::same_as<T, Long> || std::same_as<T, Float> || std::same_as<T, Double> ||
        std::same_as<T, ByteArray> || std::same_as<T, String> || std::same_as<T, IntArray> ||
        std::same_as<T, LongArray> || std::same_as<T, compound> ||
        std::same_as<T, list>;

    template<typename T>
    concept can_construct_nbt_tag = nbt_tag<std::remove_cvref_t<T>>;
    template<typename T>
    concept legacy_nbt_input_iterator = std::input_iterator<T>&&
        (can_construct_nbt_tag<typename std::iterator_traits<T>::value_type>||
        std::same_as<typename std::iterator_traits<T>::value_type, tag>)&&
        std::equality_comparable<T>;

    template<typename T>
    concept nbt_range = std::ranges::input_range<T>&&
        (can_construct_nbt_tag<typename std::ranges::range_value_t<T>>||
        std::same_as<value, std::ranges::range_value_t<T>>);

    //For now this uses a normal tag
    //Maybe a named tag is needed we will see
    class compound final : public std::unordered_map<std::string, tag>
    {
    public:

        ~compound() = default;
    };

    class list
    {
    public:
        using container = std::vector<tag>;
        using value_type = tag;
        using size_type = size_t;
        using difference_type = std::ptrdiff_t;
        using reference = tag&;
        using const_reference = const tag&;
        using pointer =  std::allocator_traits<container::allocator_type>::pointer;
        using const_pointer =  std::allocator_traits<container::allocator_type>::const_pointer;
        using iterator = container::iterator;
        using const_iterator = container::const_iterator;

        list();

        ~list() = default;

        tag_type get_list_type()const noexcept { return m_current_type; }
        void set_list_type(tag_type type)
        {
            if(m_current_type != tag_type::UNKNOWN)
                throw nbt_error("list already has a type");
            m_current_type = type;
        }

        /* ITERATORS */
        decltype(auto) begin(this auto& self) noexcept;
        decltype(auto) end(this auto& self) noexcept;
        decltype(auto) rbegin(this auto& self) noexcept;
        decltype(auto) rend(this auto& self) noexcept;

        /* ELEMENT ACCESS */
        [[nodiscard]] decltype(auto) at(this auto& self, size_t idx);
        [[nodiscard]] decltype(auto) operator[](this auto& self, size_t idx);
        [[nodiscard]] decltype(auto) front(this auto& self);
        [[nodiscard]] decltype(auto) back(this auto& self);
        [[nodiscard]] decltype(auto) data(this auto& self) noexcept;

        /* CAPACITY */
        [[nodiscard]] decltype(auto) empty(this auto& self) noexcept;
        [[nodiscard]] size_t size()const noexcept;
        [[nodiscard]] size_t max_size()const noexcept;
        void reserve(size_t new_capacity);

        /* MODIFIERS */

        void clear() noexcept;
        iterator insert(const_iterator pos, auto&& value)
            requires can_construct_nbt_tag<decltype(value)>||
            std::same_as<list::value_type, std::remove_reference_t<decltype(value)>>;

        iterator insert(const_iterator pos, size_t count, const auto& value)
            requires can_construct_nbt_tag<decltype(value)>||
            std::same_as<list::value_type, std::remove_cvref_t<decltype(value)>>;

        iterator insert(const_iterator pos, legacy_nbt_input_iterator auto begin, legacy_nbt_input_iterator auto end)
            requires std::same_as<decltype(begin), decltype(end)>;


        iterator insert_range(const_iterator pos, nbt_range auto&& range);


        iterator emplace(const_iterator pos, auto&&... args);

        iterator erase(const_iterator pos)
            { return m_data.erase(pos); }
        iterator erase(const_iterator begin, const_iterator  end)
            {return m_data.erase(begin, end);}

        void push_back(const auto& value)
            { insert(m_data.end(), value); }
        void push_back(auto&& value)
            { insert (m_data.end(), std::forward<decltype(value)>(value)); }
        reference emplace_back(auto&&... args)
            { return *emplace(m_data.end(), std::forward<decltype(args)>(args)...); }
        void append_range(auto&& range)
            { insert(m_data.end(), std::forward<decltype(range)>(range)); }
        void pop_back()
            { m_data.pop_back(); }

        // void resize(size_t new_size);
        // void resize(size_t new_size, const auto& value);
        // void swap(list& other) noexcept;

    private:
        friend std::formatter<list>;
        void resolve_tag_type_or_throw(tag_type type);
        bool check_container_for_tag_consistency(legacy_nbt_input_iterator auto begin, legacy_nbt_input_iterator auto end)
            requires std::same_as<decltype(begin), decltype(end)>;

        template<typename T>
        iterator emplace_at(const_iterator pos, T&& args)
            requires std::same_as<std::remove_cvref_t<T>, Byte> || std::same_as<std::remove_cvref_t<T>, Short>||
            std::same_as<std::remove_cvref_t<T>, Int> || std::same_as<std::remove_cvref_t<T>, Long>||
            std::same_as<std::remove_cvref_t<T>, Float> || std::same_as<std::remove_cvref_t<T>, Double>;

        template<can_construct_nbt_tag T, typename ...Args>
        iterator emplace_at(const_iterator pos, std::in_place_type_t<T> type, Args&& ...args)
            requires std::constructible_from<T, Args...>;

        template<can_construct_nbt_tag T, typename ...Args>
        iterator emplace_at(const_iterator pos, std::in_place_type_t<T> type, Args&& ...args)
        {
            throw nbt_error("cannot construct object from args");
        }


        tag_type m_current_type;
        container m_data;
    };

    class tag : public value
    {
    public:
        using value::variant;
        using value::operator=;
        tag()
            : value(std::in_place_type_t<compound>())
        {}

        virtual ~tag() = default;

        template<can_construct_nbt_tag T>
        [[nodiscard]]
        bool is() const noexcept { return std::holds_alternative<T>(*this); }

        [[nodiscard]]
        tag_type get_type()const noexcept
        {
            return std::visit([](const auto& v) noexcept { return get_tag_type(v); }, *this);
        }

        value& variant()
        {
            return *this;
        }

        const value& variant()const
        {
            return *this;
        }

        template<can_construct_nbt_tag T>
        decltype(auto) get_ref(this auto& self)
        {
            return std::get<T>(self);
        }

        template<can_construct_nbt_tag T>
        operator const T&() const
        {
            return std::get<T>(this);
        }

        decltype(auto) operator[](this auto& self, std::string key)
        {
            if (!std::holds_alternative<compound>(self))
                throw nbt_error("object is not compound");

            return std::get<compound>(self)[key];
        }
    };

    class named_tag final : public tag
    {
    public:
        named_tag() = default;

        named_tag(std::string name)
            : tag(), m_name(std::move(name)) {}

        template<can_construct_nbt_tag T>
        named_tag(std::string name, T value)
            : tag(std::move(value)), m_name(name) {}

        named_tag(const named_tag& other) = default;

        named_tag(named_tag&& other) = default;

        ~named_tag() = default;

        named_tag& operator= (const named_tag& other) = default;

        named_tag& operator= (named_tag&& other) = default;

        std::string_view name()const { return m_name; }

    private:
        std::string m_name;
    };

    /*------------ LIST IMPLEMENTATION ------------*/
    inline list::list()
        : m_current_type(tag_type::UNKNOWN), m_data(){}
    /* ITERATORS */
    decltype(auto) list::begin(this auto& self) noexcept
    {
        return self.m_data.begin();
    }

    decltype(auto) list::end(this auto& self) noexcept
    {
        return self.m_data.end();
    }

    decltype(auto) list::rbegin(this auto& self) noexcept
    {
        return self.m_data.rbegin();
    }

    decltype(auto) list::rend(this auto& self) noexcept
    {
        return self.m_data.rend();
    }

    /* ELEMENT ACCESS */
    [[nodiscard]] decltype(auto) list::at(this auto& self, size_t idx)
    {
        return self.m_data.at(idx);
    }

    [[nodiscard]] decltype(auto) list::operator[](this auto& self, size_t idx)
    {
        return self.at(idx);
    }

    [[nodiscard]] decltype(auto) list::front(this auto& self)
    {
        return self.m_data.front();
    }

    [[nodiscard]] decltype(auto) list::back(this auto& self)
    {
        return self.m_data.back();
    }

    [[nodiscard]] decltype(auto) list::data(this auto& self) noexcept
    {
        return self.m_data.data();
    }

    /* CAPACITY */
    [[nodiscard]] decltype(auto) list::empty(this auto& self) noexcept
    {
        return self.m_data.empty();
    }

    [[nodiscard]] inline size_t list::size()const noexcept
    {
        return m_data.size();
    }

    [[nodiscard]] inline size_t list::max_size()const noexcept
    {
        return m_data.max_size();
    }

    void inline list::reserve(size_t new_capacity)
    {
        m_data.reserve(new_capacity);
    }

    /* MODIFIERS */

    void inline list::clear() noexcept
    {
        m_data.clear();
    }

    list::iterator list::insert(const_iterator pos, auto&& value)
        requires can_construct_nbt_tag<decltype(value)>||
        std::same_as<list::value_type, std::remove_reference_t<decltype(value)>>
    {
        const tag_type type = get_tag_type(value);
        resolve_tag_type_or_throw(type);
        return m_data.insert(pos, std::forward<decltype(value)>(value));
    }

    list::iterator list::insert(const_iterator pos, size_t count, const auto& value)
        requires can_construct_nbt_tag<decltype(value)>||
        std::same_as<list::value_type, std::remove_cvref_t<decltype(value)>>
    {
        const tag_type type = get_tag_type(value);
        resolve_tag_type_or_throw(type);
        return m_data.insert(pos, count, std::forward<decltype(value)>(value));
    }

    list::iterator list::insert(const_iterator pos, legacy_nbt_input_iterator auto begin, legacy_nbt_input_iterator auto end)
        requires std::same_as<decltype(begin), decltype(end)>
    {
        const tag_type type = get_tag_type(*begin);
        resolve_tag_type_or_throw(type);

        if constexpr(std::same_as<typename std::iterator_traits<decltype(begin)>::value_type, list::value_type>)
            if(!check_container_for_tag_consistency(begin, end))
                throw nbt_error("inconsitency in input range");

        return m_data.insert(pos, begin, end);
    }

    list::iterator list::insert_range(const_iterator pos, nbt_range auto&& range)
    {
        const tag_type type = get_tag_type(*std::ranges::begin(range));
        resolve_tag_type_or_throw(type);

        if constexpr(std::same_as<typename std::ranges::range_value_t<decltype(range)>, list::value_type>)
            if(!check_container_for_tag_consistency(std::ranges::begin(range), std::ranges::end(range)))
                throw nbt_error("inconsitency in input range");

        return m_data.insert(pos, std::forward<decltype(range)>(range));
    }

    list::iterator list::emplace(const_iterator pos, auto&&... args)
    {
        switch(m_current_type)
        {
            case tag_type::BYTE:
                return emplace_at(pos, std::in_place_type_t<Byte>(), std::forward<decltype(args)>(args)...);
            case tag_type::SHORT:
                return emplace_at(pos, std::in_place_type_t<Short>(), std::forward<decltype(args)>(args)...);
            case tag_type::INT:
                return emplace_at(pos, std::in_place_type_t<Int>(), std::forward<decltype(args)>(args)...);
            case tag_type::LONG:
                return emplace_at(pos, std::in_place_type_t<Long>(), std::forward<decltype(args)>(args)...);
            case tag_type::FLOAT:
                return emplace_at(pos, std::in_place_type_t<Float>(), std::forward<decltype(args)>(args)...);
            case tag_type::DOUBLE:
                return emplace_at(pos, std::in_place_type_t<Double>(), std::forward<decltype(args)>(args)...);
            case tag_type::BYTE_ARRAY:
                return emplace_at(pos, std::in_place_type_t<ByteArray>(), std::forward<decltype(args)>(args)...);
            case tag_type::STRING:
                return emplace_at(pos, std::in_place_type_t<String>(), std::forward<decltype(args)>(args)...);
            case tag_type::LIST:
                return emplace_at(pos, std::in_place_type_t<list>(), std::forward<decltype(args)>(args)...);
            case tag_type::COMPOUND:
                return emplace_at(pos, std::in_place_type_t<compound>(), std::forward<decltype(args)>(args)...);
            case tag_type::INT_ARRAY:
                return emplace_at(pos, std::in_place_type_t<IntArray>(), std::forward<decltype(args)>(args)...);
            case tag_type::LONG_ARRAY:
                return emplace_at(pos, std::in_place_type_t<LongArray>(), std::forward<decltype(args)>(args)...);
            case tag_type::UNKNOWN:
                throw nbt_error("cannot emplace in list of unkown type");
            default:
                throw nbt_error("tag type not supported");
        }
    }
    //     void push_back(const auto& value) requires can_construct_nbt_tag<decltype(value)>;
    //     void push_back(auto&& value) requires can_construct_nbt_tag<decltype(value)>;
    //     list::reference emplace_back(auto&&... args);
    //     void append_range(auto&& range);
    //     void pop_back();
    //     void resize(size_t new_size);
    //     void resize(size_t new_size, const auto& value);
    //     void swap(list& other) noexcept;

    /* PRIVATE STUFF */
    inline void list::resolve_tag_type_or_throw(tag_type type)
    {
        if (m_current_type == tag_type::UNKNOWN && m_data.size() == 0)
            m_current_type = type;
        else if (m_current_type == tag_type::UNKNOWN && m_data.size() > 0)
            assert(false); // this should never happen
        else if(m_current_type != type)
            throw nbt_error("wrong tag type");
    }

    bool list::check_container_for_tag_consistency(legacy_nbt_input_iterator auto begin, legacy_nbt_input_iterator auto end)
        requires std::same_as<decltype(begin), decltype(end)>
    {
        const tag_type type = get_tag_type(*begin);

        for (auto it = begin; it != end; it++)
        {
            switch(type)
            {
                case tag_type::BYTE:
                {
                    if (!std::holds_alternative<Byte>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::SHORT:
                {
                    if (!std::holds_alternative<Short>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::INT:
                {
                    if (!std::holds_alternative<Int>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::LONG:
                {
                    if (!std::holds_alternative<Long>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::FLOAT:
                {
                    if (!std::holds_alternative<Float>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::DOUBLE:
                {
                    if (!std::holds_alternative<Double>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::BYTE_ARRAY:
                {
                    if (!std::holds_alternative<ByteArray>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::STRING:
                {
                    if (!std::holds_alternative<String>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::LIST:
                {
                    if (!std::holds_alternative<list>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::COMPOUND:
                {
                    if (!std::holds_alternative<compound>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::INT_ARRAY:
                {
                    if (!std::holds_alternative<IntArray>(it->variant()))
                        return false;
                    break;
                }
                case tag_type::LONG_ARRAY:
                {
                    if (!std::holds_alternative<LongArray>(begin->variant()))
                        return false;
                    break;
                }
                default: throw nbt_error("no such type");
            }
        }

        return true;
    }

    template<can_construct_nbt_tag T, typename ...Args>
    list::iterator list::emplace_at(const_iterator pos, std::in_place_type_t<T> type, Args&& ...args)
        requires std::constructible_from<T, Args...>
    {
        return m_data.emplace(pos, type, std::forward<Args>(args)...);
    }

    nbt parse(std::istream& data);

    inline nbt parse(std::filesystem::path file)
    {
        std::ifstream f(file.c_str(), std::ios::binary);
        return parse(f);
    }

    template<typename T>
    void serialize(std::vector<uint8_t>& buffer, T type)
        requires std::same_as<T, Short> || std::same_as<T, Int>||
        std::same_as<T, Long>
    {
        //std::byteswap(type) possibly needed
        auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(type);
        buffer.append_range(bytes);
    }

    template<typename T>
    void serialize(std::vector<uint8_t>& buffer, T type)
        requires std::same_as<T, Float> || std::same_as<T, Double>
    {
        //std::byteswap(type) possibly needed
        auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(type);
        buffer.append_range(bytes);
    }

    void serialize(std::vector<uint8_t>& buffer, std::byte b)
    {
        buffer.push_back(std::to_integer<uint>(b));
    }

    template<typename T>
    void serialize(std::vector<uint8_t>& buffer, const Array<T>& array)
        requires std::same_as<T, std::byte> || std::same_as<T, Int> || std::same_as<T, Long>
    {
        serialize(buffer, static_cast<Int>(array.size()));
        for(const auto value : array)
        {
            serialize(buffer, value);
        }
    }

    void serialize(std::vector<uint8_t>& buffer, const std::string& str)
    {
        serialize(buffer, static_cast<Short>(str.size()));
        buffer.append_range(str);
    }

    void serialize(std::vector<uint8_t>& buffer, const list& str)
    {

    }

    void serialize(std::vector<uint8_t>& buffer, const compound& str)
    {
    }

    void serialize(std::vector<uint8_t>& buffer, const tag& str)
    {

    }

} // namespace mc::NBT

//FMT FORMATTERS

template<>
struct std::formatter<mc::nbt::tag_type> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::nbt::tag_type& tag, FmtContext& ctx) const
    {

        switch(tag)
        {
            case mc::nbt::tag_type::BYTE: return std::formatter<std::string>::format("BYTE", ctx);
            case mc::nbt::tag_type::SHORT: return std::formatter<std::string>::format("SHORT", ctx);
            case mc::nbt::tag_type::INT: return std::formatter<std::string>::format("INT", ctx);
            case mc::nbt::tag_type::LONG: return std::formatter<std::string>::format("LONG", ctx);
            case mc::nbt::tag_type::FLOAT: return std::formatter<std::string>::format("FLOAT", ctx);
            case mc::nbt::tag_type::DOUBLE: return std::formatter<std::string>::format("DOUBLE", ctx);
            case mc::nbt::tag_type::BYTE_ARRAY: return std::formatter<std::string>::format("BYTE_ARRAY", ctx);
            case mc::nbt::tag_type::STRING: return std::formatter<std::string>::format("STRING", ctx);
            case mc::nbt::tag_type::LIST: return std::formatter<std::string>::format("LIST", ctx);
            case mc::nbt::tag_type::COMPOUND: return std::formatter<std::string>::format("COMPOUND", ctx);
            case mc::nbt::tag_type::INT_ARRAY: return std::formatter<std::string>::format("INT_ARRAY", ctx);
            case mc::nbt::tag_type::LONG_ARRAY: return std::formatter<std::string>::format("LONG_ARRA", ctx);
            default: return std::formatter<std::string>::format("UNKNOWN", ctx);
        }
        std::unreachable();
    }
};

template<>
struct std::formatter<mc::nbt::tag> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::nbt::tag& obj, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}", obj.variant());
    }
};

template<>
struct std::formatter<mc::nbt::named_tag> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::nbt::named_tag& obj, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}:{{{}}}", obj.name(), static_cast<const mc::nbt::tag&>(obj));
    }
};

template<>
struct std::formatter<mc::nbt::compound> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::nbt::compound& obj, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}", static_cast<const std::unordered_map<std::string, mc::nbt::tag>&>(obj));
    }
};

template<>
struct std::formatter<mc::nbt::list> : public std::formatter<std::string>
{
    template<typename FmtContext>
    FmtContext::iterator format(const mc::nbt::list& obj, FmtContext& ctx) const
    {
        return std::format_to(ctx.out(), "{{Type{}, Values:{}}}", obj.get_list_type(), obj.m_data);
    }
};


//SERIALIZERS



template<>
struct iu::Serializer<mc::nbt::list>
{
    void Serialize(std::vector<uint8_t>& buffer, const mc::nbt::list& object)
    {
    }
};

template<>
struct iu::Serializer<mc::nbt::compound>
{
    Serializer(bool is_root)
        : m_is_root(is_root) {}
    void Serialize(std::vector<uint8_t>& buffer, const mc::nbt::compound& object)
    {
        for (const auto& [name, value] : object)
        {
            buffer.push_back(static_cast<uint8_t>(value.get_type()));
            mc::nbt::serialize(buffer, name);
            Serializer<mc::nbt::tag>().Serialize(buffer, value);
        }
    }
private:
    bool m_is_root;
};

template<>
struct iu::Serializer<mc::nbt::tag>
{
    //This should be called only with root tags
    void Serialize(std::vector<uint8_t>& buffer, const mc::nbt::tag& object)
    {
        if (!object.is<mc::nbt::compound>())
            throw nbt_error("root tag needs to be of type compund");
    }

};



#endif // NBT_H