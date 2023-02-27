#ifndef _TYPE_H_
#define _TYPE_H_

#include "ast/Tag.h"
#include "IR/IRType.h"
#include <memory>
#include <string>
#include <vector>

class Ptr;


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


enum class FuncTag
{
    _inline = 1,
    _noreturn = 2
};

class FuncSpec
{
private:
    unsigned token_{};

public:
    bool IsInline() const { return token_ & static_cast<unsigned>(FuncTag::_inline); }
    bool IsNoreturn() const { return token_ & static_cast<unsigned>(FuncTag::_noreturn); }

    bool SetSpec(Tag);
};


enum class TypeTag
{
    int8, int16, int32, int64,
    uint8, uint16, uint32, uint64,
    flt32, flt64, _void, customed
};

class CPtrType;

class CType
{
public:
    virtual std::unique_ptr<CPtrType> AttachPtr(const Ptr*) const = 0;
    virtual const IRType* ToIRType(ITypePool*) const = 0;
    virtual std::string ToString() const = 0;

    virtual bool Compatible(const CType* other) const = 0;

    virtual bool IsScalar() const { return false; }
    virtual bool IsFloat() const { return false; }
    virtual bool IsInteger() const { return false; }
    virtual bool IsPtr() const { return false; }
    virtual bool IsDerived() const { return false; }
    virtual bool IsComplete() const { return false; }

    QualType Qual() const { return qual_; }
    QualType& Qual() { return qual_; }
    StorageType Storage() const { return storage_; }
    StorageType& Storage() { return storage_; }

    bool operator==(const CType&) const = delete;
    bool operator!=(const CType&) const = delete;

private:
    QualType qual_;
    StorageType storage_;
};


class ErrorType : public CType
{
public:
    std::unique_ptr<CPtrType> AttachPtr(const Ptr*) const override { return nullptr; };
    const IRType* ToIRType(ITypePool*) const override { return nullptr; };
    std::string ToString() const override { return "<error-type>"; }

    bool Compatible(const CType* other) { return false; };
};


class CArithmType : public CType
{
public:
    CArithmType(TypeTag);

    std::unique_ptr<CPtrType> AttachPtr(const Ptr*) const override;
    const IRType* ToIRType(ITypePool*) const override;

    bool Compatible(const CType*) const override;

    bool IsComplete() const override { return true; }
    bool IsInteger() const override { return unsigned(type_) <= 7; }
    bool IsScalar() const override { return type_ != TypeTag::customed; }
    bool IsFloat() const override { return type_ == TypeTag::flt32 || type_ == TypeTag::flt64; }
    bool IsUnsigned() const { return unsigned(type_) >= 4 && unsigned(type_) <= 7; }

    uint64_t Size() const { return size_; }

    bool operator>(const CArithmType&) const;
    bool operator<(const CArithmType&) const;

    std::string ToString() const override;


private:
    TypeTag type_{};
    uint64_t size_{};
};


class CFuncType : public CType
{
public:
    CFuncType() {}
    CFuncType(std::unique_ptr<CType> ret, size_t paramcount) :
        return_(std::move(ret)) { paramlist_.reserve(paramcount); }

    std::unique_ptr<CPtrType> AttachPtr(const Ptr*) const override { return nullptr; }
    std::string ToString() const override { return ""; }
    const FuncType* ToIRType(ITypePool*) const override;
    bool Compatible(const CType*) const override { return false; }

    const CType* ReturnType() const { return return_.get(); }
    auto& ReturnType() { return return_; }
    bool Variadic() const { return variadic_; }
    bool& Variadic() { return variadic_; }
    size_t ParaCount() const { return paramlist_.size(); }

    bool IsDerived() const override { return true; }
    bool IsComplete() const override { return true; }

    void AddParam(const CType* t);


private:
    bool variadic_{};
    std::vector<const CType*> paramlist_{};
    std::unique_ptr<CType> return_{};
};


class CPtrType : public CType
{
public:
    CPtrType() {}
    CPtrType(std::unique_ptr<CType> p) : point2_(std::move(p)) {}

    std::string ToString() const override { return ""; }
    std::unique_ptr<CPtrType> AttachPtr(const Ptr*) const override { return nullptr; } 
    const IRType* ToIRType(ITypePool*) const { return nullptr; }
    bool Compatible(const CType*) const { return false; }

    size_t Size() { return 8; }

    bool IsScalar() const override { return true; }
    bool IsPtr() const override { return  true; }
    bool IsDerived() const override { return true; }
    bool IsComplete() const override { return true; }

    const CType* Point2() const { return point2_.get(); }
    auto& Point2() { return point2_; }


private:
    std::unique_ptr<CType> point2_{};
};


class CVoidType : public CType
{
public:
    std::unique_ptr<CPtrType> AttachPtr(const Ptr*) const override { return nullptr; }
    const VoidType* ToIRType(ITypePool*) const override;
    std::string ToString() const override { return "void"; }

    bool Compatible(const CType* other) const override { return false; }
};

#endif // _TYPE_H_
