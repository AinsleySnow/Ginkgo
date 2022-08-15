#ifndef _PRIMARYEXPR_H_
#define _PRIMARYEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "Expression.h"
#include <memory>
#include <optional>
#include <string>

class PrimaryExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<std::string> literal{};
    std::optional<Constant> constValue{};
    std::unique_ptr<Expression> expression{};

    PrimaryExpr(std::unique_ptr<std::string>&& l) :
        literal(std::move(l)) {};
    PrimaryExpr(const Constant& c) :
        constValue(c) {};
    PrimaryExpr(std::unique_ptr<Expression>&& e) :
        expression(std::move(e)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _PRIMARYEXPR_H_
