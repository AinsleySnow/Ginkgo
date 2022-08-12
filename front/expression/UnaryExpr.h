#ifndef _UNARYEXPR_H_
#define _UNARYEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "CastExpr.h"
#include "PostfixExpr.h"
#include <memory>

class UnaryExpr : public Node, public IGenerable
{
private:
    inline IR Inc_Dec_Sizeof_UnaryExpr(SymbolTable&) const;
    inline IR UnaryOp_CastExpr(SymbolTable&) const;
    inline IR Sizeof_Alignof_TypeName(SymbolTable&) const;

public:
    Tag op;
    std::unique_ptr<PostfixExpr> postfixExpr{};
    std::unique_ptr<UnaryExpr> unaryExpr{};
    std::unique_ptr<CastExpr> castExpr{};
    std::unique_ptr<TypeName> typeName{};

    UnaryExpr(std::unique_ptr<PostfixExpr>&& pfe) : 
        postfixExpr(std::move(pfe)) {};
    UnaryExpr(Tag tag, std::unique_ptr<UnaryExpr>&& ue) : 
        op(tag), unaryExpr(std::move(ue)) {};
    UnaryExpr(Tag tag, std::unique_ptr<CastExpr>&& ce) : 
        op(tag), castExpr(std::move(ce)) {};
    UnaryExpr(Tag tag, std::unique_ptr<TypeName>&& tn) : 
        op(tag), typeName(std::move(tn)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _UNARYEXPR_H_
