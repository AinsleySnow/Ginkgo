#include "ast/CType.h"
#include "ast/Expression.h"
#include "IR/IROperand.h"
#include "messages/Error.h"
#include "visitast/ASTVisitor.h"
#include <algorithm>
#include <climits>
#include <cfloat>
#include <utf8.h>


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

ConstExpr::ConstExpr(double d) : val_(d)
{
    type_ = std::make_unique<CArithmType>(TypeTag::flt64);
}

ConstExpr::ConstExpr(bool b) : val_(b)
{
    type_ = std::make_unique<CArithmType>(TypeTag::int8);
}

ConstExpr::ConstExpr(const std::string& s)
{
    // no prefix or the prefix is u8
    if (s[0] == '\'' || s[0] == 'u' && s[1] == '8')
    {
        val_ = static_cast<uint64_t>(s[1]);
        type_ = std::make_unique<CArithmType>(TypeTag::uint8);
    }
    else if (s[0] == 'u')
    {
        auto copy = s;
        copy.erase(copy.begin());
        copy.pop_back();
        auto temp = utf8::utf8to16(copy);
        val_ = static_cast<uint64_t>(temp[0]);
        type_ = std::make_unique<CArithmType>(TypeTag::uint16);
    }
    else // the prefix is U or L; wchar_t has width of 32, as defined in stddef.h
    {
        auto copy = s;
        copy.erase(copy.begin());
        copy.pop_back();
        auto temp = utf8::utf8to32(copy);
        val_ = static_cast<uint64_t>(temp[0]);
        type_ = std::make_unique<CArithmType>(TypeTag::uint32);
    }
}

ConstExpr::ConstExpr(uint64_t u, int base, const std::string& suffix) : val_(u)
{
    auto copy = suffix;
    std::transform(copy.begin(), copy.end(), copy.begin(),
        [](char c){ return std::tolower(c); });

    if (copy.empty() && base == 10)
    {
        if (u <= INT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int32);
        else type_ = std::make_unique<CArithmType>(TypeTag::int64);
    }
    else if (copy.empty())
    {
        if (u <= INT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int32);
        else if (u <= UINT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::uint32);
        else if (u <= INT64_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int64);
        else type_ = std::make_unique<CArithmType>(TypeTag::uint64);
    }
    else if (copy == "u")
    {
        if (u <= UINT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::uint32);
        else type_ = std::make_unique<CArithmType>(TypeTag::uint64);
    }
    else if (copy == "l" && base == 10)
    {
        if (u <= INT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int32);
        else type_ = std::make_unique<CArithmType>(TypeTag::int64);
    }
    else if (copy == "l")
    {
        if (u <= INT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int32);
        else if (u <= UINT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::uint32);
        else if (u <= INT64_MAX) type_ = std::make_unique<CArithmType>(TypeTag::int64);
        else type_ = std::make_unique<CArithmType>(TypeTag::uint64);
    }
    else if (copy == "ul")
    {
        if (u <= UINT32_MAX) type_ = std::make_unique<CArithmType>(TypeTag::uint32);
        else type_ = std::make_unique<CArithmType>(TypeTag::uint64);
    }
    else if (copy == "ll" && base == 10)
        type_ = std::make_unique<CArithmType>(TypeTag::int64);
    else if (copy == "ll")
    {
        if (u <= INT64_MAX) type_ = std::make_unique<CArithmType>(TypeTag::uint32);
        else type_ = std::make_unique<CArithmType>(TypeTag::uint64);
    }
    else if (copy == "ull")
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
    return 0;
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


void DataofExpr::Accept(ASTVisitor* v)
{
    v->VisitDataofExpr(this);
}

const std::unique_ptr<Expr>& DataofExpr::ContentAsExpr() const
{
    if (std::holds_alternative<std::unique_ptr<Expr>>(content_))
        return std::get<0>(content_);
    return nullptr;
}

const std::unique_ptr<Declaration>& DataofExpr::ContentAsDecl() const
{
    if (std::holds_alternative<std::unique_ptr<Declaration>>(content_))
        return std::get<1>(content_);
    return nullptr;
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
