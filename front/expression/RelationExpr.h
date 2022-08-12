#ifndef _RELATIONEXPR_H_
#define _RELATIONEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "ShiftExpr.h"
#include <memory>

class RelationExpr : public Node, public IGenerable
{
public:
    Tag op;
    std::unique_ptr<ShiftExpr> shiftExpr{};
    std::unique_ptr<RelationExpr> relationExpr{};

    RelationExpr(std::unique_ptr<ShiftExpr>&&);
    RelationExpr(std::unique_ptr<RelationExpr>&&, Tag, std::unique_ptr<ShiftExpr>&&);

    IR Generate(SymbolTable&) const override;
};

#endif // _RELATIONEXPR_H_
