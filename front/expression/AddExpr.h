#ifndef _ADDEXPR_H_
#define _ADDEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "MultiExpr.h"
#include <memory>

class AddExpr : public Node, public IGenerable
{
public:
    Tag op;
    std::unique_ptr<AddExpr> addExpr{};
    std::unique_ptr<MultiExpr> multiExpr{};

    AddExpr(std::unique_ptr<MultiExpr>&& me) :
        multiExpr(std::move(me)) {};
    AddExpr(std::unique_ptr<AddExpr>&& ae, Tag, std::unique_ptr<MultiExpr>&& me) :
        addExpr(std::move(ae)), multiExpr(std::move(me)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _ADDEXPR_H_
