#include "ast/CType.h"
#include "ast/Expression.h"
#include "IR/IROperand.h"
#include "messages/Error.h"
#include "visitast/Visitor.h"
#include <algorithm>
#include <climits>
#include <cfloat>


void AssignExpr::Accept(Visitor* v)
{
    v->VisitAssignExpr(this);
}

void BinaryExpr::Accept(Visitor* v)
{
    v->VisitBinaryExpr(this);
}

bool BinaryExpr::IsConstant() const
{
    return Val()->IsConstant();
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

ConstExpr::ConstExpr(uint64_t u, int base, std::string suffix)
{
    val_.intgr_ = u;

    std::transform(suffix.begin(), suffix.end(), suffix.begin(),
        [](char c){ return std::tolower(c); });

    if (suffix.empty() && base == 10)
    {
        if (u <= INT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int32);
        else type_ = std::make_unique<CArithmType>(TypeTag::int64);
    }
    else if (suffix.empty())
    {
        if (u <= INT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int32);
        else if (u <= UINT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::uint32);
        else if (u <= INT64_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int64);
        else type_ = std::make_unique<CArithmType>(TypeTag::uint64);
    }
    else if (suffix == "u")
    {
        if (u <= UINT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::uint32);
        else type_ = std::make_unique<CArithmType>(TypeTag::uint64);
    }
    else if (suffix == "l" && base == 10)
    {
        if (u <= INT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int32);
        else type_ = std::make_unique<CArithmType>(TypeTag::int64);
    }
    else if (suffix == "l")
    {
        if (u <= INT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int32);
        else if (u <= UINT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::uint32);
        else if (u <= INT64_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int64);
        else type_ = std::make_unique<CArithmType>(TypeTag::uint64);
    }
    else if (suffix == "ul")
    {
        if (u <= UINT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::uint32);
        else type_ = std::make_unique<CArithmType>(TypeTag::uint64);
    }
    else if (suffix == "ll" && base == 10)
        type_ = std::make_unique<CArithmType>(TypeTag::int64);
    else if (suffix == "ll")
    {
        if (u <= INT64_MAX) type_ = std::make_unique<CArithmType>(TypeTag::uint32);
        else type_ = std::make_unique<CArithmType>(TypeTag::uint64);
    }
    else if (suffix == "ull")
        type_ = std::make_unique<CArithmType>(TypeTag::uint64);
}

ConstExpr::ConstExpr(double d, char suffix)
{
    val_.flt_ = d;
    if (std::tolower(suffix) == 'f')
        type_ = std::make_unique<CArithmType>(TypeTag::flt32);
    else if (std::tolower(suffix) == 'l')
        type_ = std::make_unique<CArithmType>(TypeTag::flt64);
    else
    {
        if (d < FLT_MAX) type_ = std::make_unique<CArithmType>(TypeTag::flt32);
        else type_ = std::make_unique<CArithmType>(TypeTag::flt64);

    }
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

bool UnaryExpr::IsConstant() const
{
    return Val()->IsConstant();
}
