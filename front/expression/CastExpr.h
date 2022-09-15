#ifndef _CASTEXPR_H_
#define _CASTEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include <memory>

class UnaryExpr;
#include "UnaryExpr.h"
class TypeName;
#include "../declaration/TypeName.h"

class CastExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<UnaryExpr> unaryExpr{};
    std::unique_ptr<TypeName> typeName{};
    std::unique_ptr<CastExpr> castExpr{};

    CastExpr(std::unique_ptr<UnaryExpr>&& ue) : 
        unaryExpr(std::move(ue)) {};
    CastExpr(std::unique_ptr<TypeName>&& tn, std::unique_ptr<CastExpr>&& ce) :
        typeName(std::move(tn)), castExpr(std::move(ce)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _CASTEXPR_H_
