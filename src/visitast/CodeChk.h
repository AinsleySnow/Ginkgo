#ifndef _CODE_CHECK_H_
#define _CODE_CHECK_H_

#include "visitast/Scope.h"
#include "visitast/ASTVisitor.h"


class CodeChk : public ASTVisitor
{
public:
    void VisitAssignExpr(AssignExpr*) override;
    void VisitBinaryExpr(BinaryExpr*) override;
    void VisitCondExpr(CondExpr*) override;
    void VisitConstant(ConstExpr*) override;
    void VisitFuncDef(FuncDef*) override;
    void VisitLogicalExpr(LogicalExpr*) override;
    void VisitObjDef(ObjDef*) override;
    void VisitUnaryExpr(UnaryExpr*) override;

    void VisitForStmt(ForStmt*) override;
    void VisitIfStmt(IfStmt*) override;
    void VisitWhileStmt(WhileStmt*) override;

private:
    CArithmType MatchArithmType(const CArithmType*, const CArithmType*);

    ScopeStack stack_{};
};

#endif // _CODE_CHECK_H_
