#ifndef _IR_TYPE_H_
#define _IR_TYPE_H_

#include <memory>
#include <string>
#include <vector>


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
    virtual const IntType* ToInteger() const { return nullptr; }
    virtual const FloatType* ToFloatPoint() const { return nullptr; }
    virtual const FuncType* ToFunction() const { return nullptr; }
    virtual const PtrType* ToPointer() const { return nullptr; }
    virtual const ArrayType* ToArray() const { return nullptr; }
    virtual const StructType* ToStruct() const { return nullptr; }
    virtual const UnionType* ToUnion() const { return nullptr; }

    virtual bool IsArithm() const { return false; }
    virtual bool IsInt() const { return false; }
    virtual bool IsFloat() const { return false; }
    virtual bool IsPtr() const { return false; }
    virtual bool IsAggerate() const { return false; }

    const static IntType* GetInt8(bool);
    const static IntType* GetInt16(bool);
    const static IntType* GetInt32(bool);
    const static IntType* GetInt64(bool);
    const static FloatType* GetFloat32();
    const static FloatType* GetFloat64();

    const static FuncType* GetFunction(const IRType*, const std::vector<const IRType*>&, bool);
    const static PtrType* GetPointer(const IRType*);
    const static ArrayType* GetArray(size_t, const IRType*);
    const static StructType* GetStruct(const std::vector<const IRType*>&);
    const static UnionType* GetUnion(const std::vector<const IRType*>&);

private:
    static std::vector<std::unique_ptr<IRType>> customedtypes_;
};


class IntType : public IRType
{
public:
    enum class intype { int8 = 8, int16 = 16, int32 = 32, int64 = 64 };
    IntType(intype t, bool s) : size_(t), signed_(s) {}

    std::string ToString() const override { return "int" + std::to_string((int)size_); }
    IntType* ToInteger() override { return this; }
    const IntType* ToInteger() const override { return this; }

    bool IsArithm() const override { return true; }
    bool IsInt() const override { return true; }

    intype GetSize() const { return size_; }
    bool IsSigned() const { return signed_; }

private:
    intype size_{};
    bool signed_{};
};

class FloatType : public IRType
{
public:
    enum class fltype { flt32, flt64 };
    FloatType(fltype t) : size_(t) {}

    std::string ToString() const override { return "int" + std::to_string((int)size_); }
    FloatType* ToFloatPoint() override { return this; }
    const FloatType* ToFloatPoint() const override { return this; }

    bool IsArithm() const override { return true; }
    bool IsFloat() const override { return true; }

private:
    fltype size_{};
};


class ArrayType : public IRType
{
public:
    ArrayType(size_t s, const IRType* t) : size_(s), type_(t) {}

    std::string ToString() const override;
    ArrayType* ToArray() override { return this; }
    const ArrayType* ToArray() const override { return this; }

private:
    size_t size_{};
    const IRType* type_{};
};

class PtrType : public IntType
{
public:
    PtrType(const IRType* t) :
        IntType(IntType::intype::int64, false), type_(t) {}
    std::string ToString() const override { return type_->ToString() + '*'; }
    PtrType* ToPointer() override { return this; }
    const PtrType* ToPointer() const override { return this; }

    const IRType* Dereference() const { return type_; }

private:
    const IRType* type_{};
};


class FuncType : public IRType
{
public:
    FuncType(const IRType* ret,
        const std::vector<const IRType*>& p, bool v) : 
        retype_(std::move(ret)), param_(p), variadic_(v) {}

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
    UnionType(const std::vector<const IRType*>& f) : fields_(f) {}
    std::string ToString() const override;
    UnionType* ToUnion() override { return this; }
    const UnionType* ToUnion() const override { return this; }

private:
    std::vector<const IRType*> fields_{};
};


#endif // _IR_TYPE_H_
