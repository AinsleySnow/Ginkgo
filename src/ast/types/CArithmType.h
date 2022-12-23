#ifndef _ARITHM_TYPE_H_
#define _ARITHM_TYPE_H_

#include "CType.h"
#include "CPtrType.h"
#include <cstdint>
#include <algorithm>
#include <memory>
#include <string>


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


class CArithmType : public CType
{
private:
    uint64_t size_{};
    unsigned spec_{};
    unsigned raw_{};

    bool Raw2Spec();


public:
    CArithmType() : size_(8), spec_(static_cast<unsigned>(Spec::int64)) {}
    CArithmType(Spec);
    CArithmType(const std::shared_ptr<CType> t) : CType(*t) {}
    CArithmType(Tag);

    bool Compatible(const CType*) const override;

    bool IsComplete() const override;
    bool IsInteger() const override;
    bool IsScalar() const override;
    bool IsFloat() const override;
    bool IsUnsigned() const;

    uint64_t GetSize() const;
    bool SetSpec(Tag);

    CArithmType* ToArithm() override { return this; }
    CPtrType* ToPtr() override { return nullptr; }

    bool operator>(const CArithmType&) const;
    bool operator<(const CArithmType&) const;

    std::string ToString() const override;
};


#endif // _ARITHM_TYPE_H_
