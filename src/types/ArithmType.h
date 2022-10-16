#ifndef _ARITHM_TYPE_H_
#define _ARITHM_TYPE_H_

#include "Type.h"
#include "PtrType.h"
#include <cstdint>
#include <algorithm>


enum class Spec
{
    int8 = 1,
    int16 = 2,
    int32 = 4,
    int64 = 8,
    float32 = 16,
    float64 = 32,
    _unsigned = 64,
    _bool = 128,
    intmask = int8 | int16 | int32 | int64,
    floatmask = float32 | float64
};


class ArithmType : public Type
{
private:
    uint64_t size_{};
    unsigned spec_{};
    unsigned raw_{};

    bool Raw2Spec();

public:
    ArithmType() : size_(8), spec_(static_cast<unsigned>(Spec::int64)) {}
    ArithmType(const Type* t) : Type(*t) {}
    ArithmType(Tag);

    bool Compatible(const Type*) const override;

    bool IsComplete() const override;
    bool IsInteger() const override;
    bool IsScalar() const override;
    bool IsFloat() const override;
    bool IsUnsigned() const;

    uint64_t GetSize() const;
    bool SetSpec(Tag);

    const ArithmType* ToArithm() const override { return this; }
    const PtrType* ToPtr() const override { return nullptr; }

    bool operator>(const ArithmType&) const;
    bool operator<(const ArithmType&) const;
};


#endif // _ARITHM_TYPE_H_
