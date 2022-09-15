#ifndef _RELATIONEXPR_H_
#define _RELATIONEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include <memory>

class ShiftExpr;
#include "ShiftExpr.h"

class RelationExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<RelationExpr> relationExpr{};
    Tag op;
    std::unique_ptr<ShiftExpr> shiftExpr{};

    RelationExpr(std::unique_ptr<ShiftExpr>&& se) :
        shiftExpr(std::move(se)) {};
    RelationExpr(std::unique_ptr<RelationExpr>&& re, Tag t, std::unique_ptr<ShiftExpr>&& se) :
        relationExpr(std::move(re)), op(t), shiftExpr(std::move(se)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _RELATIONEXPR_H_
