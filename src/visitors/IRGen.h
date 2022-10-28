#ifndef _IR_GEN_H_
#define _IR_GEN_H_

#include "Visitor.h"
#include "utils/IR.h"
#include <memory>
#include <string>


class IRGen : public Visitor
{
private:
    std::unique_ptr<IR> ir_{};

    IROper Tag2Op(Tag) const;
    inline const auto GetLastestTempVar(unsigned long) const;
    inline const auto GenerateTempVar();


public:
    void VisitAssignExpr(AssignExpr*) override;
    void VisitBinaryExpr(BinaryExpr*) override;
    void VisitCondExpr(CondExpr*) override;
    void VisitUnaryExpr(UnaryExpr*) override;
};

#endif // _IR_GEN_H_
