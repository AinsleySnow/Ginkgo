#include "ast/CType.h"
#include "ast/Expression.h"
#include "IR/IROperand.h"
#include "messages/Error.h"
#include "visitast/ASTVisitor.h"
#include <algorithm>
#include <climits>
#include <cfloat>


void ArrayExpr::Accept(ASTVisitor* v)
{
    v->VisitArrayExpr(this);
}

void AssignExpr::Accept(ASTVisitor* v)
{
    v->VisitAssignExpr(this);
}

void BinaryExpr::Accept(ASTVisitor* v)
{
    v->VisitBinaryExpr(this);
}

bool BinaryExpr::IsConstant() const
{
    return Val()->Is<Constant>();
}

void CallExpr::Accept(ASTVisitor* v)
{
    v->VisitCallExpr(this);
}

void CastExpr::Accept(ASTVisitor* v)
{
    v->VisitCastExpr(this);
}

void CondExpr::Accept(ASTVisitor* v)
{
    v->VisitCondExpr(this);
}

void ConstExpr::Accept(ASTVisitor* v)
{
    v->VisitConstant(this);
}

ConstExpr::ConstExpr(uint64_t u) : val_(u)
{
    type_ = std::make_unique<CArithmType>(TypeTag::uint64);
}

ConstExpr::ConstExpr(double d)
{
    type_ = std::make_unique<CArithmType>(TypeTag::flt64);
}

ConstExpr::ConstExpr(bool b) : val_(b)
{
    type_ = std::make_unique<CArithmType>(TypeTag::int8);
}

ConstExpr::ConstExpr(uint64_t u, int base, const std::string& suffix) : val_(u)
{
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

ConstExpr::ConstExpr(double d, char suffix) : val_(d)
{
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


uint64_t ConstExpr::GetInt() const
{
    if (std::holds_alternative<uint64_t>(val_))
        return std::get<uint64_t>(val_);
    else if (std::holds_alternative<bool>(val_))
        return std::get<bool>(val_);
    return;
}

double ConstExpr::GetFloat() const
{
    if (std::holds_alternative<double>(val_))
        return std::get<double>(val_);
    return 0;
}

bool ConstExpr::IsZero() const
{
    if (std::holds_alternative<uint64_t>(val_))
        return std::get<uint64_t>(val_) == 0;
    else if (std::holds_alternative<double>(val_))
        return std::get<double>(val_) == 0;
    else
        return !std::get<bool>(val_);
}


void EnumConst::Accept(ASTVisitor* v)
{
    v->VisitEnumConst(this);
}

void EnumList::Accept(ASTVisitor* v)
{
    v->VisitEnumList(this);
}

void EnumList::Append(std::unique_ptr<EnumConst> expr)
{
    exprlist_.push_back(std::move(expr));
}

void ExprList::Accept(ASTVisitor* v)
{
    v->VisitExprList(this);
}

void ExprList::Append(std::unique_ptr<Expr> expr)
{
    exprlist_.push_back(std::move(expr));
}

void IdentExpr::Accept(ASTVisitor* v)
{
    v->VisitIdentExpr(this);
}

void LogicalExpr::Accept(ASTVisitor* v)
{
    v->VisitLogicalExpr(this);
}

void StrExpr::Accept(ASTVisitor* v)
{
    v->VisitStrExpr(this);
}

void UnaryExpr::Accept(ASTVisitor* v)
{
    v->VisitUnaryExpr(this);
}

bool UnaryExpr::IsConstant() const
{
    return Val()->Is<Constant>();
}
