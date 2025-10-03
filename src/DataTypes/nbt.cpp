#include <bit>
#include <concepts>
#include <istream>
#include <stdexcept>
#include <string>
#include <utility>

#include "DataTypes/nbt.h"
#include "utils.h"

namespace mc::nbt
{

    void serializeString(std::vector<std::uint8_t>& buffer, const std::string &data)
    {
        util::ShortSerializer().Serialize(buffer, data.size());
        buffer.insert(buffer.end(), data.begin(), data.end());
    }

    /******************** Other ********************/

    tag_type parseTagType(std::istream& data)
    {
        char tag = std::to_underlying(tag_type::UNKNOWN);
        data.read(&tag, 1);
        return tag < -2 || tag >> 12 ? tag_type::UNKNOWN : static_cast<tag_type>(tag);
    }

    template<typename T>
    T parseNumeric(std::istream& data)
        requires std::integral<T> || std::same_as<Byte, T>
    {
        T out{0};
        data.read((char*)&out, sizeof(T));

        if constexpr(std::same_as<Byte, T>)
            return out;
        else
            return std::byteswap(out);
    }

    template<std::floating_point T>
    T parseFloat(std::istream& data)
    {
        T out;
        data.read(reinterpret_cast<char*>(&out), sizeof(T));
        return out;
    }

    std::string parseString(std::istream& data)
    {
        Short stringSize = parseNumeric<Short>(data);
        std::string out(stringSize, ' ');
        data.read(out.data(), stringSize);
        return out;
    }

    template<typename T>
    Array<T> parseArray(std::istream& data)
    {
        Array<T> out;
        Int  size = parseNumeric<Int>(data);
        out.reserve(size);
        for(Int i = 0; i < size; i++)
            out.push_back(parseNumeric<T>(data));
        return out;
    }

    list parseList(std::istream& data);

    //Probably will require more error checking in the future
    compound parseCompound(std::istream& data)
    {
        compound obj;
        while(data.good())
        {
            tag_type nextTag = parseTagType(data);

            if(nextTag == tag_type::END)
            {
                return obj;
            }

            std::string tagName = parseString(data);
            switch(nextTag)
            {
                case tag_type::BYTE:
                    obj.emplace(tagName, parseNumeric<Byte>(data));
                    break;
                case tag_type::SHORT:
                    obj.emplace(tagName, parseNumeric<Short>(data));
                    break;
                case tag_type::INT:
                    obj.emplace(tagName, parseNumeric<Int>(data));
                    break;
                case tag_type::LONG:
                    obj.emplace(tagName, parseNumeric<Long>(data));
                    break;
                case tag_type::FLOAT:
                    obj.emplace(tagName, parseFloat<Float>(data));
                    break;
                case tag_type::DOUBLE:
                    obj.emplace(tagName, parseFloat<Double>(data));
                    break;
                case tag_type::BYTE_ARRAY:
                    obj.emplace(tagName, parseArray<Byte>(data));
                    break;
                case tag_type::STRING:
                    obj.emplace(tagName, parseString(data));
                    break;
                case tag_type::LIST:
                    obj.emplace(tagName, parseList(data));
                    break;
                case tag_type::COMPOUND:
                    obj.emplace(tagName, parseCompound(data));
                    break;
                case tag_type::INT_ARRAY:
                    obj.emplace(tagName, parseArray<Int>(data));
                    break;
                case tag_type::LONG_ARRAY:
                    obj.emplace(tagName, parseArray<Long>(data));
                    break;
                default:
                    throw std::runtime_error("Unknown tag type" + std::to_string((int)nextTag));
            }
        }

        return obj;
    }

    //Probably will require more error checking in the future
    list parseList(std::istream& data)
    {
        list out;
        tag_type contained_type = parseTagType(data);
        Int size = parseNumeric<Int>(data);
        Int count = 0;
        out.set_list_type(contained_type);
        while(data.good())
        {

            if (contained_type == tag_type::END || count >= size)
            {
                return out;
            }

            switch(contained_type)
            {
                case tag_type::BYTE:
                    out.emplace_back(parseNumeric<Byte>(data));
                    break;
                case tag_type::SHORT:
                    out.emplace_back(parseNumeric<Short>(data));
                    break;
                case tag_type::INT:
                    out.emplace_back(parseNumeric<Int>(data));
                    break;
                case tag_type::LONG:
                    out.emplace_back(parseNumeric<Long>(data));
                    break;
                case tag_type::FLOAT:
                    out.emplace_back(parseFloat<Float>(data));
                    break;
                case tag_type::DOUBLE:
                    out.emplace_back(parseFloat<Double>(data));
                    break;
                case tag_type::BYTE_ARRAY:
                    out.emplace_back(parseArray<Byte>(data));
                    break;
                case tag_type::STRING:
                    out.emplace_back(parseString(data));
                    break;
                case tag_type::LIST:
                    out.emplace_back(parseList(data));
                    break;
                case tag_type::COMPOUND:
                    out.emplace_back(parseCompound(data));
                    break;
                case tag_type::INT_ARRAY:
                    out.emplace_back(parseArray<Int>(data));
                    break;
                case tag_type::LONG_ARRAY:
                    out.emplace_back(parseArray<Long>(data));
                    break;
                default:
                    throw std::runtime_error("Unknown tag type" + std::to_string(std::to_underlying(contained_type)));
            }
            ++count;
        }
        return out;
    }


    nbt parse(std::istream& data)
    {
        if (parseTagType(data) != tag_type::COMPOUND)
        {
            throw nbt_error("Failed to parse root nbt tag. Tag type is not compund");
        }

        std::string name = parseString(data);
        nbt root(parseCompound(data));

        return root;
    }
}
