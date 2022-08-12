#ifndef _EQUALEXPR_H_
#define _EQUALEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "RelationExpr.h"
#include <memory>

class EqualExpr : public Node, public IGenerable
{
public:
    Tag op;
    std::unique_ptr<RelationExpr> relationExpr{};
    std::unique_ptr<EqualExpr> equalExpr{};

    EqualExpr(std::unique_ptr<RelationExpr>&& re) :
        relationExpr(std::move(re)) {};
    EqualExpr(std::unique_ptr<EqualExpr>&& ee, Tag, std::unique_ptr<RelationExpr>&& re) :
        equalExpr(std::move(ee)), relationExpr(std::move(re)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _EQUALEXPR_H_
