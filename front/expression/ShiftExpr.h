#ifndef _SHIFTEXPR_H_
#define _SHIFTEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "AddExpr.h"
#include <memory>

class ShiftExpr : public Node, public IGenerable
{
public:
    Tag op;
    std::unique_ptr<AddExpr> addExpr{};
    std::unique_ptr<ShiftExpr> shiftExpr{};

    ShiftExpr(std::unique_ptr<AddExpr>&& ae) :
        addExpr(std::move(ae)) {};
    ShiftExpr(std::unique_ptr<ShiftExpr>&& se, Tag tag, std::unique_ptr<AddExpr>&& ae) :
        shiftExpr(std::move(se)), op(tag), addExpr(std::move(ae)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _SHIFTEXPR_H_
