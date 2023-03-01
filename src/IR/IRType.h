#ifndef _IR_TYPE_H_
#define _IR_TYPE_H_

#include <memory>
#include <string>
#include <vector>

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
    virtual ~IRType() {}
    virtual std::string ToString() const { return ""; }

    virtual IntType* ToInteger() { return nullptr; }
    virtual FloatType* ToFloatPoint() { return nullptr; }
    virtual FuncType* ToFunction() { return nullptr; }
    virtual PtrType* ToPointer() { return nullptr; }
    virtual ArrayType* ToArray() { return nullptr; }
    virtual StructType* ToStruct() { return nullptr; }
    virtual UnionType* ToUnion() { return nullptr; }
    virtual VoidType* ToVoid() { return nullptr; }
    virtual const IntType* ToInteger() const { return nullptr; }
    virtual const FloatType* ToFloatPoint() const { return nullptr; }
    virtual const FuncType* ToFunction() const { return nullptr; }
    virtual const PtrType* ToPointer() const { return nullptr; }
    virtual const ArrayType* ToArray() const { return nullptr; }
    virtual const StructType* ToStruct() const { return nullptr; }
    virtual const UnionType* ToUnion() const { return nullptr; }
    virtual const VoidType* ToVoid() const { return nullptr; }

    virtual bool IsArithm() const { return false; }
    virtual bool IsInt() const { return false; }
    virtual bool IsFloat() const { return false; }
    virtual bool IsPtr() const { return false; }
    virtual bool IsVoid() const { return false; }
    virtual bool IsAggerate() const { return false; }

    size_t Size() const { return size_; }
    size_t Align() const { return align_; }

    bool operator<(const IRType& rhs) const;
    bool operator>(const IRType& rhs) const;
    virtual bool operator==(const IRType& rhs) const;

protected:
    size_t size_{};
    size_t align_{};
};


class ITypePool
{
public:
    virtual ~ITypePool() {}
    void AddIRType(std::unique_ptr<IRType>);
    void MergeTypePool(ITypePool*);

private:
    std::vector<std::unique_ptr<IRType>> typelist_{};
};


class IntType : public IRType
{
public:
    const static IntType* GetInt8(bool);
    const static IntType* GetInt16(bool);
    const static IntType* GetInt32(bool);
    const static IntType* GetInt64(bool);
    IntType(size_t s, bool si) : signed_(si) { size_ = s; }

    std::string ToString() const override;
    IntType* ToInteger() override { return this; }
    const IntType* ToInteger() const override { return this; }

    bool IsArithm() const override { return true; }
    bool IsInt() const override { return true; }

    bool IsSigned() const { return signed_; }


private:
    bool signed_{};
};

class FloatType : public IRType
{
public:
    const static FloatType* GetFloat32();
    const static FloatType* GetFloat64();
    FloatType(size_t s) { size_ = s; }

    std::string ToString() const override;
    FloatType* ToFloatPoint() override { return this; }
    const FloatType* ToFloatPoint() const override { return this; }

    bool IsArithm() const override { return true; }
    bool IsFloat() const override { return true; }
};


class ArrayType : public IRType
{
public:
    static ArrayType* GetArrayType(ITypePool*, size_t, const IRType*);
    ArrayType(size_t s, const IRType* t) : type_(t) { size_ = s; }

    std::string ToString() const override;
    ArrayType* ToArray() override { return this; }
    const ArrayType* ToArray() const override { return this; }


private:
    const IRType* type_{};
};

class PtrType : public IntType
{
public:
    static PtrType* GetPtrType(ITypePool*, const IRType*);
    PtrType(const IRType* t) :
        IntType(8, false), type_(t) {}

    std::string ToString() const override;
    PtrType* ToPointer() override { return this; }
    const PtrType* ToPointer() const override { return this; }

    const IRType* Point2() const { return type_; }


private:
    const IRType* type_{};
};


class FuncType : public IRType
{
public:
    static FuncType* GetFuncType(ITypePool*, const IRType*, bool);
    FuncType(const IRType* ret, bool v) : 
        retype_(std::move(ret)), variadic_(v) { size_ = -1; }

    auto ReturnType() const { return retype_; }
    const auto& ParamType() const { return param_; }
    void AddParam(const IRType*);

    std::string ToString() const override;
    FuncType* ToFunction() override { return this; }
    const FuncType* ToFunction() const override { return this; }


private:
    const IRType* retype_{};
    std::vector<const IRType*> param_{};
    bool variadic_{};
};


class StructType : public IRType
{
public:
    static StructType* GetStructType(ITypePool*, const std::vector<const IRType*>&);
    StructType(const std::vector<const IRType*>& f) : fields_(f) {}

    std::string ToString() const override;
    StructType* ToStruct() override { return this; }
    const StructType* ToStruct() const override { return this; }

private:
    std::vector<const IRType*> fields_{};
};


class UnionType : public IRType
{
public:
    static UnionType* GetUnionType(ITypePool*, const std::vector<const IRType*>&);
    UnionType(const std::vector<const IRType*>& f) : fields_(f) {}

    std::string ToString() const override;
    UnionType* ToUnion() override { return this; }
    const UnionType* ToUnion() const override { return this; }

private:
    std::vector<const IRType*> fields_{};
};


class VoidType : public IRType
{
public:
    static const VoidType* GetVoidType();

    std::string ToString() const override { return "void"; }
    VoidType* ToVoid() override { return this; }
    const VoidType* ToVoid() const override { return this; }
};


#endif // _IR_TYPE_H_
