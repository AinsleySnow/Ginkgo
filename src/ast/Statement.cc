#include "ast/Statement.h"
#include "IR/Instr.h"
#include "visitast/ASTVisitor.h"


void Statement::PushBrInstr(Instr* i)
{
    nextlist_.push_back(static_cast<BrInstr*>(i));
}


void BreakStmt::Accept(ASTVisitor* v)
{
    v->VisitBreakStmt(this);
}

void CaseStmt::Accept(ASTVisitor* v)
{
    v->VisitCaseStmt(this);
}

void CompoundStmt::Accept(ASTVisitor* v)
{
    v->VisitCompoundStmt(this);
}

void CompoundStmt::Append(std::unique_ptr<Statement> stmt)
{
    stmtlist_.push_back(std::move(stmt));
}

void ContinueStmt::Accept(ASTVisitor* v)
{
    v->VisitContinueStmt(this);
}

void DeclStmt::Accept(ASTVisitor* v)
{
    v->VisitDeclStmt(this);
}

void DoWhileStmt::Accept(ASTVisitor* v)
{
    v->VisitDoWhileStmt(this);
}

void ExprStmt::Accept(ASTVisitor* v)
{
    v->VisitExprStmt(this);
}

void ForStmt::Accept(ASTVisitor* v)
{
    v->VisitForStmt(this);
}

void GotoStmt::Accept(ASTVisitor* v)
{
    v->VisitGotoStmt(this);
}

void IfStmt::Accept(ASTVisitor* v)
{
    v->VisitIfStmt(this);
}

void LabelStmt::Accept(ASTVisitor* v)
{
    v->VisitLabelStmt(this);
}

void RetStmt::Accept(ASTVisitor* v)
{
    v->VisitRetStmt(this);
}

void SwitchStmt::Accept(ASTVisitor* v)
{
    v->VisitSwitchStmt(this);
}

void TransUnit::Accept(ASTVisitor* v)
{
    v->VisitTransUnit(this);
}

void TransUnit::AddDecl(std::unique_ptr<DeclStmt> decl)
{
    declist_.push_back(std::move(decl));
}

void WhileStmt::Accept(ASTVisitor* v)
{
    v->VisitWhileStmt(this);
}
