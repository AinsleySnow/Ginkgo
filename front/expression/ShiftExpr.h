#ifndef _SHIFTEXPR_H_
#define _SHIFTEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include <memory>

class AddExpr;
#include "AddExpr.h"

class ShiftExpr : public Node, public IGenerable
{
public:   
    std::unique_ptr<ShiftExpr> shiftExpr{};
    Tag op;
    std::unique_ptr<AddExpr> addExpr{};

    ShiftExpr(std::unique_ptr<AddExpr>&& ae) :
        addExpr(std::move(ae)) {};
    ShiftExpr(std::unique_ptr<ShiftExpr>&& se, Tag tag, std::unique_ptr<AddExpr>&& ae) :
        shiftExpr(std::move(se)), op(tag), addExpr(std::move(ae)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _SHIFTEXPR_H_
