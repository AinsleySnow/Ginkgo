#ifndef _AST_VISITOR_H_
#define _AST_VISITOR_H_

class ArrayDef;
class Declaration;
class DeclList;
class DeclSpec;
class FuncDef;
class HeterList;
class ObjDef;
class ParamList;
class PtrDef;

class AccessExpr;
class ArrayExpr;
class AssignExpr;
class BinaryExpr;
class CallExpr;
class CastExpr;
class CondExpr;
class ConstExpr;
class DataofExpr;
class EnumConst;
class EnumList;
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


class ASTVisitor
{
public:
    virtual ~ASTVisitor() {}

    virtual void VisitArrayDef(ArrayDef*) {}
    virtual void VisitDeclList(DeclList*) {}
    virtual void VisitDeclSpec(DeclSpec*) {}
    virtual void VisitFuncDef(FuncDef*) {}
    virtual void VisitHeterList(HeterList*) {}
    virtual void VisitObjDef(ObjDef*) {}
    virtual void VisitParamList(ParamList*) {}
    virtual void VisitPtrDef(PtrDef*) {}

    virtual void VisitAccessExpr(AccessExpr*) {}
    virtual void VisitArrayExpr(ArrayExpr*) {}
    virtual void VisitAssignExpr(AssignExpr*) {}
    virtual void VisitBinaryExpr(BinaryExpr*) {}
    virtual void VisitCallExpr(CallExpr*) {}
    virtual void VisitCastExpr(CastExpr*) {}
    virtual void VisitCondExpr(CondExpr*) {}
    virtual void VisitConstant(ConstExpr*) {}
    virtual void VisitDataofExpr(DataofExpr*) {}
    virtual void VisitEnumConst(EnumConst*) {}
    virtual void VisitEnumList(EnumList*) {}
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

#endif // _AST_VISITOR_H_
