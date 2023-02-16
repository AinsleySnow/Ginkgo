#ifndef _VISITOR_H_
#define _VISITOR_H_

class Declaration;
class DeclList;
class DeclSpec;
class FuncDef;
class ObjDef;
class ParamList;

class AssignExpr;
class BinaryExpr;
class CallExpr;
class CastExpr;
class CondExpr;
class ConstExpr;
class ExprList;
class IdentExpr;
class LogicalExpr;
class StrExpr;
class UnaryExpr;

class BreakStmt;
class CaseStmt;
class CompoundStmt;
class ContinueStmt;
class DeclStmt;
class DoWhileStmt;
class ExprStmt;
class ForStmt;
class GotoStmt;
class IfStmt;
class LabelStmt;
class RetStmt;
class SwitchStmt;
class TransUnit;
class WhileStmt;


class Visitor
{
public:
    virtual ~Visitor() {}

    virtual void VisitDeclList(DeclList*) {}
    virtual void VisitDeclSpec(DeclSpec*) {}
    virtual void VisitFuncDef(FuncDef*) {}
    virtual void VisitObjDef(ObjDef*) {}
    virtual void VisitParamList(ParamList*) {}

    virtual void VisitAssignExpr(AssignExpr*) {}
    virtual void VisitBinaryExpr(BinaryExpr*) {}
    virtual void VisitCallExpr(CallExpr*) {}
    virtual void VisitCastExpr(CastExpr*) {}
    virtual void VisitCondExpr(CondExpr*) {}
    virtual void VisitConstant(ConstExpr*) {}
    virtual void VisitExprList(ExprList*) {}
    virtual void VisitIdentExpr(IdentExpr*) {}
    virtual void VisitLogicalExpr(LogicalExpr*) {}
    virtual void VisitStrExpr(StrExpr*) {}
    virtual void VisitUnaryExpr(UnaryExpr*) {}

    virtual void VisitBreakStmt(BreakStmt*) {}
    virtual void VisitCaseStmt(CaseStmt*) {}
    virtual void VisitCompoundStmt(CompoundStmt*) {}
    virtual void VisitContinueStmt(ContinueStmt*) {}
    virtual void VisitDeclStmt(DeclStmt*) {}
    virtual void VisitDoWhileStmt(DoWhileStmt*) {}
    virtual void VisitExprStmt(ExprStmt*) {}
    virtual void VisitForStmt(ForStmt*) {}
    virtual void VisitGotoStmt(GotoStmt*) {}
    virtual void VisitIfStmt(IfStmt*) {}
    virtual void VisitLabelStmt(LabelStmt*) {}
    virtual void VisitRetStmt(RetStmt*) {}
    virtual void VisitSwitchStmt(SwitchStmt*) {}
    virtual void VisitTransUnit(TransUnit*) {}
    virtual void VisitWhileStmt(WhileStmt*) {}
};

#endif // _VISITOR_H_
