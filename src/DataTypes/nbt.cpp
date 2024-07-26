#include "DataTypes/nbt.h"
#include "SFW/LoggerManager.h"
#include "utils.h"
#include <format>
#include <fstream>
#include <istream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
namespace mc::NBT
{

    void serializeString(std::vector<std::uint8_t>& buffer, const std::string &data)
    {
        util::ShortSerializer().Serialize(buffer, data.size());
        buffer.insert(buffer.end(), data.begin(), data.end());
    }

    NBTTag::NBTTag(TagType type)
        : m_tagType(type)
    {}

    NBTTag::NBTTag()
        : m_tagType(TagType::UNKNOWN)
    {}

    NBTCompound::NBTCompound(const NBTCompound& other)
    {
        Copy(other);
    }

    NBTCompound& NBTCompound::operator=(const NBTCompound& other)
    {
        Copy(other);
        return *this;
    }

    void NBTCompound::Copy(const NBTCompound& other)
    {
        for(auto& tag : other.m_objectsTree)
        {
            m_objectsTree.insert(std::make_pair(tag.first, tag.second->Clone()));
        }
    }
    /* ***************************** NBTList ***************************** */

    NBTList::Iterator::Iterator(TagPtr* it)
        : m_it(it)
    {
    }

    NBTList::ConstIterator::ConstIterator(const TagPtr* it)
        : m_it(it)
    {
    }

    NBTList::NBTList()
        : m_tagsType(TagType::UNKNOWN)
    {
    }

    NBTList::NBTList(TagType type)
        : m_tagsType(type)
    {
    }

    NBTList::NBTList(const NBTList& other)
        : NBTList()
    {
        Assign(other);
    }

    NBTList& NBTList::operator=(const NBTList &other)
    {
        Assign(other);
        return *this;
    }

    NBTTag& NBTList::operator[](size_t pos)
    {
        ASSERT(pos < Size(), "Out of bounds access");
        return *m_objectsList[pos].get();
    }
         
    const NBTTag& NBTList::operator[](size_t pos) const
    {
        ASSERT(pos < Size(), "Out of bounds access");
        return *m_objectsList[pos].get();
    }

    void NBTList::Assign(const NBTList& other)
    {
        m_tagsType = other.m_tagsType;
        
        //Deallocate any existing objects
        if (m_objectsList.size() > 0)
            m_objectsList.clear();
        
        if(other.m_objectsList.size() == 0)
            return;
        
        for(auto& obj : other.m_objectsList)
        {
            m_objectsList.emplace_back(obj->Clone());
        }
    }
  
    /******************** Other ********************/

    TagType parseTagType(std::istream& data)
    {
        char tag = (char)TagType::UNKNOWN;
        data.get(tag);
        return (TagType)tag;
    }

    template<mc::util::Numeric T>
    T parseNumeric(std::istream& data)
    {
        T out = 0;
        data.read((char*)&out, sizeof(T));
        char* asBytes = (char*)&out;

        for (size_t k = 0; k < sizeof(T)/2; k++)
            std::swap(asBytes[k], asBytes[sizeof(T) - k - 1]);

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

    NBTList parseList(std::istream& data);

    //Probably will require more error checking in the future
    NBTCompound parseCompound(std::istream& data)
    {
        NBTCompound obj;
        while(data.good())
        {
            TagType nextTag = parseTagType(data);

            if(nextTag == TagType::END)
            {
                return obj;
            }

            std::string tagName = parseString(data);
            switch(nextTag)
            {
                case TagType::BYTE:
                    obj.Insert(tagName, parseNumeric<Byte>(data));
                    break;
                case TagType::SHORT:
                    obj.Insert(tagName, parseNumeric<Short>(data));
                    break;
                case TagType::INT:
                    obj.Insert(tagName, parseNumeric<Int>(data));
                    break;
                case TagType::LONG:
                    obj.Insert(tagName, parseNumeric<Long>(data));
                    break;
                case TagType::FLOAT:
                    obj.Insert(tagName, parseNumeric<Float>(data));
                    break;
                case TagType::DOUBLE:
                    obj.Insert(tagName, parseNumeric<Double>(data));
                    break;
                case TagType::BYTE_ARRAY:
                    obj.Insert(tagName, parseArray<Byte>(data));
                    break;
                case TagType::STRING:
                    obj.Insert(tagName, parseString(data));
                    break;
                case TagType::LIST:
                    obj.Insert(tagName, parseList(data));
                    break;
                case TagType::COMPOUND:
                    obj.Insert(tagName, parseCompound(data));
                    break;
                case TagType::INT_ARRAY:
                    obj.Insert(tagName, parseArray<Int>(data));
                    break;
                case TagType::LONG_ARRAY:
                    obj.Insert(tagName, parseArray<Long>(data));
                    break;
                default:
                    throw std::runtime_error("Unknown tag type" + std::to_string((int)nextTag));
            }
        }

        return obj;
    }

    //Probably will require more error checking in the future
    NBTList parseList(std::istream& data)
    {
        NBTList out;
        TagType containedType = parseTagType(data);
        Int size = parseNumeric<Int>(data);
        Int count = 0;
        while(data.good())
        {

            if (containedType == TagType::END || count >= size)
            {
                return out;
            }

            switch(containedType)
            {
                case TagType::BYTE:
                    out.Insert(parseNumeric<Byte>(data));
                    break;
                case TagType::SHORT:
                    out.Insert(parseNumeric<Short>(data));
                    break;
                case TagType::INT:
                    out.Insert(parseNumeric<Int>(data));
                    break;
                case TagType::LONG:
                    out.Insert(parseNumeric<Long>(data));
                    break;
                case TagType::FLOAT:
                    out.Insert(parseNumeric<Float>(data));
                    break;
                case TagType::DOUBLE:
                    out.Insert(parseNumeric<Double>(data));
                    break;
                case TagType::BYTE_ARRAY:
                    out.Insert(parseArray<Byte>(data));
                    break;
                case TagType::STRING:
                    out.Insert(parseString(data));
                    break;
                case TagType::LIST:
                    out.Insert(parseList(data));
                    break;
                case TagType::COMPOUND:
                    out.Insert(parseCompound(data));
                    break;
                case TagType::INT_ARRAY:
                    out.Insert(parseArray<Int>(data));
                    break;
                case TagType::LONG_ARRAY:
                    out.Insert(parseArray<Long>(data));
                    break;
                default:
                    throw std::runtime_error("Unknown tag type" + std::to_string((int)containedType));
            }
            ++count;
        }
        return out;
    }


    NBT parse(std::istream& data)
    {
        if (parseTagType(data) != TagType::COMPOUND)
        {
            throw std::runtime_error("Root component is not Compund tag");
        }

        std::string name = parseString(data);
        NBT root(name, parseCompound(data));

        return root;
    }
}
