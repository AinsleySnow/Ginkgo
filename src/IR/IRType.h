#ifndef _IR_TYPE_H_
#define _IR_TYPE_H_

#include "utils/DynCast.h"
#include "utils/Pool.h"
#include <memory>
#include <string>
#include <vector>

class IntConst;
class IntType;
class FloatType;
class FuncType;
class PtrType;
class ArrayType;
class StructType;
class UnionType;
class VoidType;


class IRType
{
public:
    enum class TypeId { none, _int, fp, array, ptr, func, _struct, _union, _void };
    static bool ClassOf(const IRType* const) { return true; }
    TypeId id_ = TypeId::none;

    ENABLE_IS;
    ENABLE_AS;

    virtual ~IRType() {}
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
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::_int; }

    const static IntType* GetInt8(bool);
    const static IntType* GetInt16(bool);
    const static IntType* GetInt32(bool);
    const static IntType* GetInt64(bool);
    IntType(size_t s, bool si) : signed_(si) { size_ = s; }

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
    FloatType(size_t s) { size_ = s; }

    std::string ToString() const override;
};


class ArrayType : public IRType
{
public:
    static bool ClassOf(const ArrayType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::array; }

    static ArrayType* GetArrayType(Pool<IRType>*, size_t, const IRType*);
    ArrayType(size_t, const IRType*);

    std::string ToString() const override;

    auto ArrayOf() const { return type_; }
    size_t Count() const { return count_; }
    size_t& Count() { return count_; }
    bool VarlableLen() const { return variable_; }
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
    PtrType(const IRType* t) :
        IntType(8, false), type_(t) {}

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
        retype_(std::move(ret)), variadic_(v) { size_ = -1; }

    auto ReturnType() const { return retype_; }
    const auto& ParamType() const { return param_; }
    void AddParam(const IRType*);

    std::string ToString() const override;

private:
    const IRType* retype_{};
    std::vector<const IRType*> param_{};
    bool variadic_{};
};


class StructType : public IRType
{
public:
    static bool ClassOf(const StructType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::_struct; }

    static StructType* GetStructType(Pool<IRType>*, const std::vector<const IRType*>&);
    StructType(const std::vector<const IRType*>& f) : fields_(f) {}

    std::string ToString() const override;

private:
    std::vector<const IRType*> fields_{};
};


class UnionType : public IRType
{
public:
    static bool ClassOf(const UnionType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::_union; }

    static UnionType* GetUnionType(Pool<IRType>*, const std::vector<const IRType*>&);
    UnionType(const std::vector<const IRType*>& f) : fields_(f) {}

    std::string ToString() const override;

private:
    std::vector<const IRType*> fields_{};
};


class VoidType : public IRType
{
public:
    static bool ClassOf(const VoidType* const) { return true; }
    static bool ClassOf(const IRType* const i) { return i->id_ == TypeId::_void; }

    static const VoidType* GetVoidType();

    std::string ToString() const override { return "void"; }
};


#endif // _IR_TYPE_H_
