#ifndef _MULTIPLIEXPR_H_
#define _MULTIPLIEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "CastExpr.h"
#include <memory>

class MultiExpr : public Node, public IGenerable
{
public:
    Tag op;
    std::unique_ptr<CastExpr> castExpr{};
    std::unique_ptr<MultiExpr> multiExpr{};

    MultiExpr(std::unique_ptr<CastExpr>&& ce) : 
        castExpr(std::move(ce)) {};
    MultiExpr(std::unique_ptr<MultiExpr>&& me, Tag tag, std::unique_ptr<CastExpr>&& ce) :
        op(tag), castExpr(std::move(ce)), multiExpr(std::move(me)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _MULTIPLIEXPR_H_
