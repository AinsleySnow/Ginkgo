#ifndef _CONSTANT_H_
#define _CONSTANT_H_

#include "Expr.h"
#include "Tag.h"
#include "types/ArithmType.h"
#include "visitors/Visitor.h"
#include <cstdint>
#include <memory>


class Constant : public Expr
{
private:
    union
    {
        uint64_t intgr_;
        double flt_;
    } val_;


public:
    Constant() {}
    explicit Constant(uint64_t u)
    {
        val_.intgr_ = u;
        type_ = std::make_shared<ArithmType>();
    }
    explicit Constant(double d)
    {
        val_.flt_ = d;
        type_ = std::make_shared<ArithmType>();
    }
    explicit Constant(bool b)
    {
        val_.intgr_ = b ? 1 : 0;
        type_ = std::make_shared<ArithmType>();
    }
    ~Constant() {}

    uint64_t GetInt() const { return val_.intgr_; }
    double GetFloat() const { return val_.flt_; }

    void Accept(Visitor* v) override { v->VisitConstant(this); }
    Constant* ToConstant() override { return this; }

    static bool DoCalc(Tag, const Constant*, Constant&);
    static bool DoCalc(Tag, const Constant*, const Constant*, Constant&);
};

#endif // _CONSTANT_H_
