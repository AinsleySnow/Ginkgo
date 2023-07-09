#include "ast/CType.h"
#include "ast/Expression.h"
#include "IR/IROperand.h"
#include "messages/Error.h"
#include "visitast/ASTVisitor.h"
#include <algorithm>
#include <climits>
#include <cfloat>
#include <utf8.h>


void AccessExpr::Accept(ASTVisitor* v)
{
    v->VisitAccessExpr(this);
}

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

static void WashEscape(std::string& s)
{
    int start = s.find_first_of('\'');
    if (s[++start] != '\\')
        return;

    auto replace = [&s, &start](char c){
        //       start
        //         v
        // u 8 ' \ a '
        //       |
        //--lhs--+-rhs--
        auto lhs = s.substr(0, start - 1);
        s = lhs + c + '\'';
    };

    switch (s[++start])
    {
    case 'a': replace('\a'); break; case 'b': replace('\b'); break;
    case 'f': replace('\f'); break; case 'n': replace('\n'); break;
    case 'r': replace('\r'); break; case 't': replace('\t'); break;
    case 'v': replace('\v'); break; case '\'': replace('\''); break;
    case '"': replace('\"'); break; case '\\': replace('\\'); break;
    case '\?': replace('?'); break;
    case 'x':
        {
            start += 1;
            auto c = static_cast<char>(std::stoi(
                s.substr(start, s.length() - start - 1), nullptr, 16));
            s = s.substr(0, start - 2) + c + '\'';
            break;
        }
    default: // an oct number
        {
            auto c = static_cast<char>(std::stoi(
                s.substr(start, s.length() - start - 1), nullptr, 8));
            s = s.substr(0, start - 1) + c + '\'';
            break;
        }
    }
}

ConstExpr::ConstExpr(const std::string& cs)
{
    auto s = cs;
    WashEscape(s);

    // no prefix or the prefix is u8
    if (s[0] == '\'' || s[0] == 'u' && s[1] == '8')
    {
        val_ = static_cast<uint64_t>(s[1]);
        type_ = std::make_unique<CArithmType>(TypeTag::uint8);
    }
    else if (s[0] == 'u')
    {
        auto copy = s;
        copy.erase(copy.begin(), copy.begin() + 2);
        copy.pop_back();
        auto temp = utf8::utf8to16(copy);
        val_ = static_cast<uint64_t>(temp[0]);
        type_ = std::make_unique<CArithmType>(TypeTag::uint16);
    }
    else // the prefix is U or L; wchar_t has width of 32, as defined in stddef.h
    {
        auto copy = s;
        copy.erase(copy.begin(), copy.begin() + 2);
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


void SzAlgnExpr::Accept(ASTVisitor* v)
{
    v->VisitSzAlgnExpr(this);
}

Expr* SzAlgnExpr::ContentAsExpr()
{
    if (std::holds_alternative<std::unique_ptr<Expr>>(content_))
        return std::get<0>(content_).get();
    return nullptr;
}

Declaration* SzAlgnExpr::ContentAsDecl()
{
    if (std::holds_alternative<std::unique_ptr<Declaration>>(content_))
        return std::get<1>(content_).get();
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
