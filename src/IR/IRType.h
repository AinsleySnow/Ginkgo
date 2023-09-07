#ifndef _IR_TYPE_H_
#define _IR_TYPE_H_

#include "utils/DynCast.h"
#include "utils/Pool.h"
#include <memory>
#include <string>
#include <vector>

class IntConst;


class IRType
{
public:
    enum class TypeId { none, _int, fp, array, ptr, func, _struct, _union, _void };
    static bool ClassOf(const IRType* const) { return true; }
    TypeId id_ = TypeId::none;

    ENABLE_IS;
    ENABLE_AS;

    IRType(TypeId i) : id_(i) {}

    virtual std::string ToString() const { return ""; }

    bool IsArithm() const { return id_ == TypeId::_int || id_ == TypeId::fp; }

    size_t Size() const { return size_; }
    size_t Align() const { return align_; }

    bool operator<(const IRType& rhs) const;
    bool operator>(const IRType& rhs) const;
    virtual bool operator==(const IRType& rhs) const;

protected:
    size_t size_{};
    size_t align_{};
};


class IntType : public IRType
{
public:
    static bool ClassOf(const IntType* const) { return true; }
    static bool ClassOf(const IRType* const i)
    { return i->id_ == TypeId::_int || i->id_ == TypeId::ptr; }

    const static IntType* GetInt8(bool);
    const static IntType* GetInt16(bool);
    const static IntType* GetInt32(bool);
    const static IntType* GetInt64(bool);
    const static IntType* GetInt8(Pool<IRType>*, size_t, bool);
    const static IntType* GetInt16(Pool<IRType>*, size_t, bool);
    const static IntType* GetInt32(Pool<IRType>*, size_t, bool);
    const static IntType* GetInt64(Pool<IRType>*, size_t, bool);

    IntType(size_t s, bool si) :
        IRType(TypeId::_int), signed_(si) { size_ = s; align_ = s; }
    IntType(size_t s, size_t a, bool si) :
        IRType(TypeId::_int), signed_(si) { size_ = s; align_ = a; }

    std::string ToString() const override;
    bool IsSigned() const { return signed_; }


private:
    bool signed_{};
};

class FloatType : public IRType
{
public:
    static bool ClassOf(const FloatType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::fp; }

    const static FloatType* GetFloat32();
    const static FloatType* GetFloat64();
    const static FloatType* GetFloat32(Pool<IRType>*, size_t);
    const static FloatType* GetFloat64(Pool<IRType>*, size_t);

    FloatType(size_t s) : IRType(TypeId::fp) { size_ = s; align_ = s; }
    FloatType(size_t s, size_t a) : IRType(TypeId::fp) { size_ = s; align_ = a; }

    std::string ToString() const override;
};


class ArrayType : public IRType
{
public:
    static bool ClassOf(const ArrayType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::array; }

    static ArrayType* GetArrayType(Pool<IRType>*, size_t, const IRType*);
    static ArrayType* GetArrayType(Pool<IRType>*, size_t, size_t, const IRType*);

    ArrayType(size_t, const IRType*);
    ArrayType(size_t, size_t, const IRType*);

    std::string ToString() const override;

    auto ArrayOf() const { return type_; }
    size_t Count() const { return count_; }
    size_t& Count() { return count_; }
    bool VariableLen() const { return variable_; }
    bool& VariableLen() { return variable_; }
    bool Static() const { return static_; }
    bool& Static() { return static_; }

private:
    const IRType* type_{};

    size_t count_{};
    bool variable_{};
    bool static_{};
};

class PtrType : public IntType
{
public:
    static bool ClassOf(const PtrType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::ptr; }

    static PtrType* GetPtrType(Pool<IRType>*, const IRType*);
    static PtrType* GetPtrType(Pool<IRType>*, size_t, const IRType*);

    PtrType(const IRType* t) : IntType(8, false), type_(t) { id_ = TypeId::ptr; }
    PtrType(size_t a, const IRType* t) : IntType(8, a, false), type_(t) { id_ = TypeId::ptr; }

    std::string ToString() const override;
    const IRType* Point2() const { return type_; }

private:
    const IRType* type_{};
};


class FuncType : public IRType
{
public:
    static bool ClassOf(const FuncType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::func; }

    static FuncType* GetFuncType(Pool<IRType>*, const IRType*, bool);
    FuncType(const IRType* ret, bool v) :
        IRType(TypeId::func), retype_(ret), variadic_(v) { size_ = -1; }
    FuncType(const IRType* ret, std::initializer_list<const IRType*> l, bool v) :
        IRType(TypeId::func), retype_(ret), param_(l), variadic_(v) { size_ = -1; }

    auto ReturnType() const { return retype_; }
    const auto& ParamType() const { return param_; }
    void AddParam(const IRType*);
    bool Variadic() const { return variadic_; }

    std::string ToString() const override;

private:
    const IRType* retype_{};
    std::vector<const IRType*> param_{};
    bool variadic_{};
};


// 'Heter' is short for 'heterogeneous'
class HeterType : public IRType
{
public:
    static bool ClassOf(const HeterType* const) { return true; }
    static bool ClassOf(const IRType* const i)
    { return i->id_ == TypeId::_struct || i->id_ == TypeId::_union; }

    HeterType(TypeId i, const std::string& n, size_t s, size_t a) :
        IRType(i), name_(n) { size_ = s; align_ = a; }

    auto begin() { return fields_.begin(); }
    auto end() { return fields_.end(); }
    auto begin() const { return fields_.cbegin(); }
    auto end() const { return fields_.cend(); }
    auto cbegin() const { return fields_.cbegin(); }
    auto cend() const { return fields_.cend(); }

    auto Name() const { return name_; }
    auto FieldNum() const { return fields_.size(); }
    void AddField(const IRType* t, size_t o) { fields_.push_back(std::make_pair(t, o)); }
    auto At(int i) const { return fields_.at(i); }

protected:
    std::string name_{};
    std::vector<std::pair<const IRType*, size_t>> fields_{};
};


class StructType : public HeterType
{
public:
    static bool ClassOf(const StructType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::_struct; }

    static StructType* GetStructType(Pool<IRType>*, const std::string&, size_t, size_t);
    StructType(const std::string& n, size_t s, size_t a) :
        HeterType(TypeId::_struct, n, s, a) {}
    StructType(const std::string& n, size_t s, size_t a,
        std::initializer_list<std::pair<const IRType*, size_t>> l) :
        HeterType(TypeId::_struct, n, s, a)
    {
        for (auto [ty, off] : l)
            AddField(ty, off);
    }

    std::string ToString() const override;
};


class UnionType : public HeterType
{
public:
    static bool ClassOf(const UnionType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::_union; }

    static UnionType* GetUnionType(Pool<IRType>*, const std::string&, size_t, size_t);
    UnionType(const std::string& n, size_t s, size_t a) :
        HeterType(TypeId::_union, n, s, a) {}
    UnionType(const std::string& n, size_t s, size_t a,
        std::initializer_list<std::pair<const IRType*, size_t>> l) :
        HeterType(TypeId::_union, n, s, a)
    {
        for (auto [ty, off] : l)
            AddField(ty, off);
    }

    std::string ToString() const override;
};


class VoidType : public IRType
{
public:
    static bool ClassOf(const VoidType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::_void; }

    static const VoidType* GetVoidType();
    VoidType() : IRType(TypeId::_void) {}

    std::string ToString() const override { return "void"; }
};


#endif // _IR_TYPE_H_
