#include "NBT/nbt.h"

namespace mc
{
    namespace NBT
    {
        NBTTag::NBTTag(NBTTagType type)
            : m_tagType(type)
        {}

        NBTTag::NBTTag()
            : m_tagType(NBTTagType::UNKNOWN)
        {}
    }
}