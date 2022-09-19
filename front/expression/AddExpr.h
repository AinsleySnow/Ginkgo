#ifndef _ADDEXPR_H_
#define _ADDEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include <memory>

class MultiExpr;
#include "MultiExpr.h"

class AddExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<AddExpr> addExpr{};
    Tag op;
    std::unique_ptr<MultiExpr> multiExpr{};

    AddExpr(std::unique_ptr<MultiExpr>&& me) :
        multiExpr(std::move(me)) {};
    AddExpr(std::unique_ptr<AddExpr>&& ae, Tag t, std::unique_ptr<MultiExpr>&& me) :
        addExpr(std::move(ae)), op(t), multiExpr(std::move(me)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _ADDEXPR_H_
