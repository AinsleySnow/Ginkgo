#ifndef _CONSTANT_H_
#define _CONSTANT_H_

#include "Expr.h"
#include "ArithmType.h"
#include "Tag.h"
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
    Constant() : Expr(Which::constant) {}
    explicit Constant(uint64_t u) : Expr(Which::constant)
    {
        val_.intgr_ = u;
        type_ = std::make_shared<ArithmType>();
    }
    explicit Constant(double d) : Expr(Which::constant)
    {
        val_.flt_ = d;
        type_ = std::make_shared<ArithmType>();
    }
    explicit Constant(bool b) : Expr(Which::constant)
    {
        val_.intgr_ = b ? 1 : 0;
        type_ = std::make_shared<ArithmType>();
    }
    ~Constant() {}

    uint64_t GetInt() const { return val_.intgr_; }
    double GetFloat() const { return val_.flt_; }

    void Accept(Visitor* v) { v->VisitConstant(this); }

    static bool DoCalc(Tag, const Constant&, Constant&);
    static bool DoCalc(Tag, const Constant&, const Constant&, Constant&);
};

#endif // _CONSTANT_H_
