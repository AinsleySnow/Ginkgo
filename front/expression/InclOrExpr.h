#ifndef _INCLOREXPR_H_
#define _INCLOREXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "XorExpr.h"
#include <memory>

class InclOrExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<XorExpr> xorExpr{};
    std::unique_ptr<InclOrExpr> inclOrExpr{};

    InclOrExpr(std::unique_ptr<XorExpr>&& xe) :
        xorExpr(std::move(xe)) {};
    InclOrExpr(std::unique_ptr<XorExpr>&& xe, std::unique_ptr<InclOrExpr>&& ior) :
        xorExpr(std::move(xe)), inclOrExpr(std::move(ior)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _INCLOREXPR_H_
