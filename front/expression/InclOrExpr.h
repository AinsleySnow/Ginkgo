#ifndef _INCLOREXPR_H_
#define _INCLOREXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include <memory>

class XorExpr;
#include "XorExpr.h"

class InclOrExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<InclOrExpr> inclOrExpr{};
    std::unique_ptr<XorExpr> xorExpr{};

    InclOrExpr(std::unique_ptr<XorExpr>&& xe) :
        xorExpr(std::move(xe)) {};
    InclOrExpr(std::unique_ptr<InclOrExpr>&& ior, std::unique_ptr<XorExpr>&& xe) :
        inclOrExpr(std::move(ior)), xorExpr(std::move(xe)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _INCLOREXPR_H_
