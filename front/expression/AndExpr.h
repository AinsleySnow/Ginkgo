#ifndef _ANDEXPR_H_
#define _ANDEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "EqualExpr.h"
#include <memory>

class AndExpr : public Node, public IGenerable
{
public:
    Tag op;
    std::unique_ptr<EqualExpr> equalExpr{};
    std::unique_ptr<AndExpr> andExpr{};

    AndExpr(std::unique_ptr<EqualExpr>&& ee) :
        equalExpr(std::move(ee)) {};
    AndExpr(std::unique_ptr<AndExpr>&& ae, Tag tag, std::unique_ptr<EqualExpr>&& ee) :
        andExpr(std::move(ae)), op(tag), equalExpr(std::move(ee)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _ANDEXPR_H_
