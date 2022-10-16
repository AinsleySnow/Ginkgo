#include "Type.h"

bool QualType::SetToken(Tag t)
{
    switch (t)
    {
        case Tag::_const:
            token_ |= static_cast<unsigned>(QualTag::_const); return true;
        case Tag::_restrict:
            token_ |= static_cast<unsigned>(QualTag::_const); return true;
        case Tag::_volatile:
            token_ |= static_cast<unsigned>(QualTag::_const); return true;
        case Tag::_atomic:
            token_ |= static_cast<unsigned>(QualTag::_const); return true;
    }
    return false;
}


bool StorageType::SetToken(Tag t)
{
    if (token_) return false;
    switch (t)
    {
        case Tag::_static:
            token_ |= static_cast<unsigned>(StorageTag::_static); return true;
        case Tag::_extern:
            token_ |= static_cast<unsigned>(StorageTag::_extern); return true;
        case Tag::_typedef:
            token_ |= static_cast<unsigned>(StorageTag::_typedef); return true;
        case Tag::_thread_local:
            token_ |= static_cast<unsigned>(StorageTag::_thread_local); return true;
        case Tag::_auto:
            token_ |= static_cast<unsigned>(StorageTag::_auto); return true;
        case Tag::_register:
            token_ |= static_cast<unsigned>(StorageTag::_register); return true;
    }
    return false;
}
