#include "ast/CType.h"
#include "ast/Expression.h"
#include "messages/Error.h"
#include "visitast/Visitor.h"
#include <climits>


void AssignExpr::Accept(Visitor* v)
{
    v->VisitAssignExpr(this);
}

void BinaryExpr::Accept(Visitor* v)
{
    v->VisitBinaryExpr(this);
}

void CallExpr::Accept(Visitor* v)
{
    v->VisitCallExpr(this);
}

void CastExpr::Accept(Visitor* v)
{
    v->VisitCastExpr(this);
}

void CondExpr::Accept(Visitor* v)
{
    v->VisitCondExpr(this);
}

void ConstExpr::Accept(Visitor* v)
{
    v->VisitConstant(this);
}

ConstExpr::ConstExpr(uint64_t u)
{
    val_.intgr_ = u;
    type_ = std::make_unique<CArithmType>(TypeTag::uint64);
}

ConstExpr::ConstExpr(double d)
{
    val_.flt_ = d;
    type_ = std::make_unique<CArithmType>(TypeTag::flt64);
}

ConstExpr::ConstExpr(bool b)
{
    val_.intgr_ = b ? 1 : 0;
    type_ = std::make_unique<CArithmType>(TypeTag::int8);
}


void ExprList::Accept(Visitor* v)
{
    v->VisitExprList(this);
}

void ExprList::Append(std::unique_ptr<Expr> expr)
{
    exprlist_.push_back(std::move(expr));
}

void IdentExpr::Accept(Visitor* v)
{
    v->VisitIdentExpr(this);
}

void LogicalExpr::Accept(Visitor* v)
{
    v->VisitLogicalExpr(this);
}

void StrExpr::Accept(Visitor* v)
{
    v->VisitStrExpr(this);
}

void UnaryExpr::Accept(Visitor* v)
{
    v->VisitUnaryExpr(this);
}

