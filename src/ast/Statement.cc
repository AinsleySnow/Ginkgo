#include "ast/Statement.h"
#include "visitast/Visitor.h"


void BreakStmt::Accept(Visitor* v)
{
    v->VisitBreakStmt(this);
}

void CaseStmt::Accept(Visitor* v)
{
    v->VisitCaseStmt(this);
}

void CompoundStmt::Accept(Visitor* v)
{
    v->VisitCompoundStmt(this);
}

void CompoundStmt::Append(std::unique_ptr<Statement> stmt)
{
    stmtlist_.push_back(std::move(stmt));
}

void ContinueStmt::Accept(Visitor* v)
{
    v->VisitContinueStmt(this);
}

void DeclStmt::Accept(Visitor* v)
{
    v->VisitDeclStmt(this);
}

void DoWhileStmt::Accept(Visitor* v)
{
    v->VisitDoWhileStmt(this);
}

void ExprStmt::Accept(Visitor* v)
{
    v->VisitExprStmt(this);
}

void ForStmt::Accept(Visitor* v)
{
    v->VisitForStmt(this);
}

void GotoStmt::Accept(Visitor* v)
{
    v->VisitGotoStmt(this);
}

void IfStmt::Accept(Visitor* v)
{
    v->VisitIfStmt(this);
}

void LabelStmt::Accept(Visitor* v)
{
    v->VisitLabelStmt(this);
}

void RetStmt::Accept(Visitor* v)
{
    v->VisitRetStmt(this);
}

void SwitchStmt::Accept(Visitor* v)
{
    v->VisitSwitchStmt(this);
}

void WhileStmt::Accept(Visitor* v)
{
    v->VisitWhileStmt(this);
}
