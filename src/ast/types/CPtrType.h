#ifndef _PTR_TYPE_H_
#define _PTR_TYPE_H_

#include "CType.h"
#include <memory>


class CPtrType : public CType
{
public:
    CPtrType() {}
    CPtrType(std::shared_ptr<CType> p) : point2_(p) {}

    size_t GetSize() { return 8; }

    bool IsScalar() const override { return true; }
    bool IsPtr() const override { return  true; }
    bool IsDerived() const override { return true; }
    bool IsComplete() const override { return true; }

    void SetPoint2(std::shared_ptr<CType> t) { point2_ = t; }
    auto GetPoint2() { return point2_; }

    CPtrType* ToPtr() override { return this; }


private:
    std::shared_ptr<CType> point2_{};
};

#endif // _PTR_TYPE_H_
