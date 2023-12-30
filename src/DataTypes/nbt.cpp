#include "DataTypes/nbt.h"
#include <functional>
namespace mc
{
    namespace NBT
    {
        NBTTag::NBTTag(TagType type)
            : m_tagType(type)
        {}

        NBTTag::NBTTag()
            : m_tagType(TagType::UNKNOWN)
        {}

        NBTCompoundObject::NBTCompoundObject(const NBTCompoundObject& other)
        {
            for(auto& tag : other.m_objectsTree)
            {
                m_objectsTree.insert(std::make_pair(tag.first, tag.second->Clone()));
            }
        }

        NBTCompoundObject::~NBTCompoundObject()
        {
            for(auto tag : m_objectsTree)
            {
                delete tag.second;
            }
        }
        /* ***************************** NBTListObject ***************************** */

        NBTListObject::Iterator::Iterator(TagPtr* it)
            : m_it(it)
        {
        }

        NBTListObject::ConstIterator::ConstIterator(const TagPtr* it)
            : m_it(it)
        {
        }

        NBTListObject::NBTListObject()
            : m_tagsType(TagType::UNKNOWN)
        {
        }

        NBTListObject::NBTListObject(TagType type)
            : m_tagsType(type)
        {
        }

        NBTListObject::NBTListObject(const NBTListObject& other)
            : NBTListObject()
        {
            Assign(other);
        }

        NBTListObject& NBTListObject::operator=(const NBTListObject &other)
        {
            Assign(other);
            return *this;
        }

        NBTTag& NBTListObject::operator[](size_t pos)
        {
            ASSERT(pos < Size(), "Out of bounds access");
            return *m_objectsList[pos].get();
        }
         
        const NBTTag& NBTListObject::operator[](size_t pos) const
        {
            ASSERT(pos < Size(), "Out of bounds access");
            return *m_objectsList[pos].get();
        }

        void NBTListObject::Assign(const NBTListObject& other)
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
}
