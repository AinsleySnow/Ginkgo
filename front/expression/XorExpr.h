#ifndef _XOREXPR_H_
#define _XOREXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include <memory>

class AndExpr;
#include "AndExpr.h"

class XorExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<XorExpr> xorExpr{};
    std::unique_ptr<AndExpr> andExpr{};
    
    XorExpr(std::unique_ptr<AndExpr>&& ae) :
        andExpr(std::move(ae)) {};
    XorExpr(std::unique_ptr<XorExpr>&& xe, std::unique_ptr<AndExpr>&& ae) :
        xorExpr(std::move(xe)), andExpr(std::move(ae)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _XOREXPR_H_
