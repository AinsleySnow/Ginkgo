#ifndef _TYPE_H_
#define _TYPE_H_

#include "ast/Tag.h"
#include "IR/IRType.h"
#include "utils/DynCast.h"
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
    int8 = 1, int16 = 2, int32 = 4, int64 = 8,
    uint8 = 16, uint16 = 32, uint32 = 64, uint64 = 128,
    flt32 = 256, flt64 = 512, _void = 1024, _enum = 2048,
    _struct = 4096, _union = 8192, _typedef = 16384,

    integer = 255, unsign = 240, scalar = 1023,
    customed = 31744
};


class CType
{
protected:
    enum class CTypeId { none, arithm, pointer, array, function, _enum, _void, error };
    static bool ClassOf(const CType const*) { return true; }
    CTypeId id_ = CTypeId::none;

public:
    CType(CTypeId id) : id_(id) {}

    virtual const IRType* ToIRType(Pool<IRType>*) const = 0;
    virtual std::string ToString() const = 0;

    CTypeId ID() const { return id_; }

    ENABLE_IS;
    ENABLE_AS;

    virtual bool Compatible(const CType* other) const = 0;
    virtual bool IsScalar() const { return false; }
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
protected:
    static bool ClassOf(const ErrorType const*) { return true; }
    static bool ClassOf(const CType const* t) { return t->ID() == CTypeId::error; }

public:
    ErrorType() : CType(CTypeId::error) {}

    const IRType* ToIRType(Pool<IRType>*) const override { return nullptr; };
    std::string ToString() const override { return "<error-type>"; }

    bool Compatible(const CType* other) { return false; };
};


class CArithmType : public CType
{
protected:
    static bool ClassOf(const CArithmType const*) { return true; }
    static bool ClassOf(const CType const* t) { return t->ID() == CTypeId::arithm; }

public:
    CArithmType(TypeTag);

    const IRType* ToIRType(Pool<IRType>*) const override;
    std::string ToString() const override;

    bool Compatible(const CType*) const override;

    bool IsComplete() const override { return true; }
    bool IsInteger() const { return unsigned(type_) & unsigned(TypeTag::integer); }
    bool IsScalar() const override { return unsigned(type_) & unsigned(TypeTag::scalar); }
    bool IsFloat() const { return type_ == TypeTag::flt32 || type_ == TypeTag::flt64; }
    bool IsUnsigned() const { return unsigned(type_) & unsigned(TypeTag::unsign); }

    uint64_t Size() const { return size_; }

    bool operator>(const CArithmType&) const;
    bool operator<(const CArithmType&) const;

private:
    TypeTag type_{};
    uint64_t size_{};
};


class CFuncType : public CType
{
protected:
    static bool ClassOf(const CFuncType const*) { return true; }
    static bool ClassOf(const CType const* t) { return t->ID() == CTypeId::function; }

public:
    CFuncType() : CType(CTypeId::function) {}
    CFuncType(std::unique_ptr<CType> ret, size_t paramcount) :
        CType(CTypeId::function), return_(std::move(ret)) { paramlist_.reserve(paramcount); }

    std::string ToString() const override { return ""; }
    const FuncType* ToIRType(Pool<IRType>*) const override;

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
    int index_{};
    bool variadic_{};
    bool inline_{};
    bool noreturn_{};
    std::vector<const CType*> paramlist_{};
    std::unique_ptr<CType> return_{};
};


class CPtrType : public CType
{
protected:
    static bool ClassOf(const CPtrType const*) { return true; }
    static bool ClassOf(const CType const* t) { return t->ID() == CTypeId::pointer; }

public:
    CPtrType() : CType(CTypeId::pointer) {}
    CPtrType(std::unique_ptr<CType> p) :
        CType(CTypeId::pointer), point2_(std::move(p)) {}

    std::string ToString() const override;
    const PtrType* ToIRType(Pool<IRType>*) const override;
    bool Compatible(const CType*) const { return false; }

    size_t Size() { return 8; }

    bool IsScalar() const override { return true; }
    bool IsComplete() const override { return true; }

    const CType* Point2() const { return point2_.get(); }
    auto& Point2() { return point2_; }


private:
    std::unique_ptr<CType> point2_{};
};


class CArrayType : public CType
{
protected:
    static bool ClassOf(const CArrayType const*) { return true; }
    static bool ClassOf(const CType const* t) { return t->ID() == CTypeId::array; }

public:
    CArrayType(std::unique_ptr<CType> ty) :
        CType(CTypeId::array), arrayof_(std::move(ty)) {}
    CArrayType(std::unique_ptr<CType> ty, size_t c) :
        CType(CTypeId::array), arrayof_(std::move(ty)), count_(c) {}

    std::string ToString() const;
    const ArrayType* ToIRType(Pool<IRType>*) const override;
    bool Compatible(const CType*) const { return false; }

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


class Member;

class CEnumType : public CType
{
protected:
    static bool ClassOf(const CEnumType const*) { return true; }
    static bool ClassOf(const CType const* t) { return t->ID() == CTypeId::_enum; }

public:
    CEnumType(const std::string& n) : CType(CTypeId::_enum),
        name_(n), underlying_(std::make_shared<CArithmType>(TypeTag::int32)) {}
    CEnumType(const std::string& n, std::shared_ptr<CType> ty) :
        CType(CTypeId::_enum), name_(n), underlying_(ty) {}

    std::string ToString() const override;
    bool Compatible(const CType*) const override { return false; }

    const IntType* ToIRType(Pool<IRType>*) const override;

    void Reserve(size_t size) { members_.reserve(size); }
    void AddMember(const Member* m) { members_[index_++] = m; }

    std::string Name() const { return name_; }
    const CType* Underlying() const { return underlying_.get(); }

private:
    int index_{};
    std::string name_{};
    std::shared_ptr<CType> underlying_{};
    std::vector<const Member*> members_{};
};


class CVoidType : public CType
{
protected:
    static bool ClassOf(const CVoidType const*) { return true; }
    static bool ClassOf(const CType const* t) { return t->ID() == CTypeId::_void; }

public:
    CVoidType() : CType(CTypeId::_void) {}

    const VoidType* ToIRType(Pool<IRType>*) const override;
    std::string ToString() const override { return "void"; }

    bool Compatible(const CType* other) const override { return false; }
};

#endif // _TYPE_H_
