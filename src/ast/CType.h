#ifndef _TYPE_H_
#define _TYPE_H_

#include "ast/Tag.h"
#include "IR/IRType.h"
#include <memory>
#include <string>
#include <vector>


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
    virtual const IRType* ToIRType(MemPool<IRType>*) const = 0;
    virtual std::string ToString() const = 0;

    virtual bool Compatible(const CType* other) const = 0;

    virtual bool IsScalar() const { return false; }
    virtual bool IsFloat() const { return false; }
    virtual bool IsInteger() const { return false; }
    virtual bool IsPtr() const { return false; }
    virtual bool IsArray() const { return false; }
    virtual bool IsVoid() const { return false; }
    virtual bool IsFunc() const { return false; }
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
    const IRType* ToIRType(MemPool<IRType>*) const override { return nullptr; };
    std::string ToString() const override { return "<error-type>"; }

    bool Compatible(const CType* other) { return false; };
};


class CArithmType : public CType
{
public:
    CArithmType(TypeTag);

    const IRType* ToIRType(MemPool<IRType>*) const override;
    std::string ToString() const override;

    bool Compatible(const CType*) const override;

    bool IsComplete() const override { return true; }
    bool IsInteger() const override { return unsigned(type_) <= 7; }
    bool IsScalar() const override { return type_ != TypeTag::customed; }
    bool IsFloat() const override { return type_ == TypeTag::flt32 || type_ == TypeTag::flt64; }
    bool IsUnsigned() const { return unsigned(type_) >= 4 && unsigned(type_) <= 7; }

    uint64_t Size() const { return size_; }

    bool operator>(const CArithmType&) const;
    bool operator<(const CArithmType&) const;

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

    std::string ToString() const override { return ""; }
    const FuncType* ToIRType(MemPool<IRType>*) const override;

    bool IsFunc() const override { return true; }
    bool Compatible(const CType*) const override { return false; }

    const CType* ReturnType() const { return return_.get(); }
    auto& ReturnType() { return return_; }

    bool Variadic() const { return variadic_; }
    bool& Variadic() { return variadic_; }
    bool Inline() const { return inline_; }
    bool& Inline() { return inline_; }
    bool Noreturn() const { return noreturn_; }
    bool& Noreturn() { return noreturn_; }

    size_t ParaCount() const { return paramlist_.size(); }

    bool IsComplete() const override { return true; }

    void AddParam(const CType* t);


private:
    bool variadic_{};
    bool inline_{};
    bool noreturn_{};
    std::vector<const CType*> paramlist_{};
    std::unique_ptr<CType> return_{};
};


class CPtrType : public CType
{
public:
    CPtrType() {}
    CPtrType(std::unique_ptr<CType> p) : point2_(std::move(p)) {}

    std::string ToString() const override;
    const PtrType* ToIRType(MemPool<IRType>*) const override;
    bool Compatible(const CType*) const { return false; }

    size_t Size() { return 8; }

    bool IsScalar() const override { return true; }
    bool IsPtr() const override { return  true; }
    bool IsComplete() const override { return true; }

    const CType* Point2() const { return point2_.get(); }
    auto& Point2() { return point2_; }


private:
    std::unique_ptr<CType> point2_{};
};


class CArrayType : public CType
{
public:
    CArrayType(std::unique_ptr<CType> ty) :
        arrayof_(std::move(ty)) {}
    CArrayType(std::unique_ptr<CType> ty, size_t c) :
        arrayof_(std::move(ty)), count_(c) {}

    std::string ToString() const;
    const ArrayType* ToIRType(MemPool<IRType>*) const override;
    bool Compatible(const CType*) const { return false; }

    bool IsArray() const override { return true; }

    bool VarlableLen() const { return variable_; }
    bool& VariableLen() { return variable_; }
    bool Static() const { return static_; }
    bool& Static() { return static_; }

private:
    std::unique_ptr<CType> arrayof_{};
    size_t count_{};
    bool variable_{};
    bool static_{};
};


class CVoidType : public CType
{
public:
    const VoidType* ToIRType(MemPool<IRType>*) const override;
    std::string ToString() const override { return "void"; }

    bool IsVoid() const override { return true; }
    bool Compatible(const CType* other) const override { return false; }
};

#endif // _TYPE_H_
