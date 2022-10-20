#ifndef _TYPE_H_
#define _TYPE_H_

#include "ast/Tag.h"

class ArithmType;
class PtrType;


enum class QualTag
{
    _const = 1,
    _restrict = 2,
    _volatile = 4,
    _atomic = 8
};


class QualType
{
private:
    unsigned token_{};

public:
    bool IsConstant() const { return token_ & static_cast<unsigned>(QualTag::_const); }
    bool IsRestrict() const { return token_ & static_cast<unsigned>(QualTag::_restrict); }
    bool IsVolatile() const { return token_ & static_cast<unsigned>(QualTag::_volatile); }
    bool IsAtomic() const { return token_ & static_cast<unsigned>(QualTag::_atomic); }

    bool SetToken(Tag);
};

enum class StorageTag
{
    _static = 1,
    _extern = 2,
    _typedef = 4,
    _thread_local = 8,
    _auto = 16,
    _register = 32
};


class StorageType
{
private:
    unsigned token_{};

public:
    bool IsStatic() const { return token_ & static_cast<unsigned>(StorageTag::_static); }
    bool IsExtern() const { return token_ & static_cast<unsigned>(StorageTag::_extern); }
    bool IsTypedef() const { return token_ & static_cast<unsigned>(StorageTag::_typedef); }
    bool IsThreadLocal() const { return token_ & static_cast<unsigned>(StorageTag::_thread_local); }
    bool IsAuto() const { return token_ & static_cast<unsigned>(StorageTag::_auto); }
    bool IsRegister() const { return token_ & static_cast<unsigned>(StorageTag::_register); }

    bool SetToken(Tag);
};


class Type
{
private:
    QualType qual_;
    StorageType storage_;


public:
    virtual bool Compatible(const Type* other) const { return false; }

    virtual bool IsScalar() const { return false; }
    virtual bool IsFloat() const { return false; }
    virtual bool IsInteger() const { return false; }
    virtual bool IsPtr() const { return false; }
    virtual bool IsDerived() const { return false; }
    virtual bool IsComplete() const { return false; }

    QualType GetQual() const { return qual_; }
    QualType& GetQual() { return qual_; }
    StorageType GetStorage() const { return storage_; }
    StorageType& GetStorage() { return storage_; }

    bool operator==(const Type&) const = delete;
    bool operator!=(const Type&) const = delete;

    virtual ArithmType* ToArithm() { return nullptr; }
    virtual PtrType* ToPtr() { return nullptr; }
};

#endif // _TYPE_H_
