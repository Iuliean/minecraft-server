#include "DataTypes/nbt.h"
#include "utils.h"
#include <iterator>
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
}
