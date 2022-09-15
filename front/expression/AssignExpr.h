#ifndef _ASSIGNEXPR_H_
#define _ASSIGNEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"

class UnaryExpr;
class CondExpr;
#include "CondExpr.h"
#include "UnaryExpr.h"

class AssignExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<CondExpr> condExpr{};
    std::unique_ptr<UnaryExpr> unaryExpr{};
    Tag op;
    std::unique_ptr<AssignExpr> assignExpr{};

    AssignExpr(std::unique_ptr<CondExpr>&& ce) :
        condExpr(std::move(ce)) {}
    AssignExpr(std::unique_ptr<UnaryExpr>&& ue, Tag t, std::unique_ptr<AssignExpr>&& ae) :
        unaryExpr(std::move(ue)), op(t), assignExpr(std::move(ae)) {}
    AssignExpr(const AssignExpr&);

    IR Generate(SymbolTable&) const override;
};

#endif // _ASSIGNEXPR_H_
