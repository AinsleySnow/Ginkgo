#ifndef _POSTFIXEXPR_H_
#define _POSTFIXEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "ArgvExprList.h"
#include "../declaration/InitList.h"
#include "../declaration/TypeName.h"

class Expression;
#include "Expression.h"
class PrimaryExpr;
#include "PrimaryExpr.h"

class PostfixExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<PrimaryExpr> primaryExpr{};
    std::unique_ptr<PostfixExpr> postfixExpr{};
    std::unique_ptr<Expression> expression{};
    std::unique_ptr<ArgvExprList> argvExprList{};
    Tag op; // ++ or -- or . or -> or left or right
    std::unique_ptr<std::string> field{};
    std::unique_ptr<TypeName> typeName{};
    std::unique_ptr<InitList> initList{};

    PostfixExpr(std::unique_ptr<PrimaryExpr>&& pe) :
        primaryExpr(std::move(pe)) {};
    PostfixExpr(std::unique_ptr<PostfixExpr>&& pe, std::unique_ptr<Expression>&& e) :
        postfixExpr(std::move(pe)), expression(std::move(e)) {};
    PostfixExpr(std::unique_ptr<PostfixExpr>&& pe) :
        postfixExpr(std::move(pe)) {};
    PostfixExpr(std::unique_ptr<PostfixExpr>&& pe, std::unique_ptr<ArgvExprList>&& ael) :
        postfixExpr(std::move(pe)), argvExprList(std::move(ael)) {};
    PostfixExpr(std::unique_ptr<PostfixExpr>&& pe, Tag t, std::unique_ptr<std::string>&& s) :
        postfixExpr(std::move(pe)), op(t), field(std::move(s)) {};
    PostfixExpr(std::unique_ptr<TypeName>&& tn, std::unique_ptr<InitList>&& il) :
        typeName(std::move(tn)), initList(std::move(il)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _POSTFIXEXPR_H_
