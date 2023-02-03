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

bool ConstExpr::DoCalc(Tag op, const ConstExpr* num, ConstExpr& output)
{
    switch (op)
    {
    case Tag::dec: case Tag::inc: case Tag::arrow:
    case Tag::_and: case Tag::dot:
        Error(ErrorId::needlval); return false;
    case Tag::plus:
        output = ConstExpr(num->GetInt());
        return true;
    case Tag::minus:
        if (num->Type()->IsFloat())
            output = ConstExpr(-num->GetFloat());
        else output = ConstExpr(-num->GetInt());
        return true;
    case Tag::exclamation:
        if (num->GetInt()) output = ConstExpr((uint64_t)0);
        else output = ConstExpr((uint64_t)1);
        return true;
    case Tag::tilde:
        if (num->Type()->IsFloat())
        {
            Error(ErrorId::operatormisuse);
            return false;
        }
        output = ConstExpr(~num->GetInt());
        return true;
    default: return false;
    }
}

bool ConstExpr::DoCalc(Tag op, const ConstExpr* left, const ConstExpr* right, ConstExpr& output)
{
#define both_float (left->Type())->IsFloat() && (right->Type())->IsFloat()
#define both_int (left->Type())->IsInteger() && (right->Type())->IsInteger()
#define int_float (left->Type())->IsInteger() && (right->Type())->IsFloat()
#define float_int (left->Type())->IsFloat() && (right->Type())->IsInteger()
#define handle_int(sym)                                         \
    if (both_int)                                               \
    {                                                           \
        output = ConstExpr(left->GetInt() sym right->GetInt()); \
        return true;                                            \
    }                                                           \
    else                                                        \
    {                                                           \
        Error(ErrorId::operatormisuse);                         \
        return false;                                           \
    }

#define handle_calc(sym)                                          \
    if (both_float)                                               \
    {                                                             \
        output = ConstExpr(left->GetFloat() sym right->GetFloat());\
        return true;                                              \
    }                                                             \
    else if (both_int)                                            \
    {                                                             \
        output = ConstExpr(left->GetInt() sym right->GetInt());    \
        return true;                                              \
    }                                                             \
    else if (int_float)                                           \
    {                                                             \
        output = ConstExpr(left->GetInt() sym right->GetFloat());  \
        return true;                                              \
    }                                                             \
    else if (float_int)                                           \
    {                                                             \
        output = ConstExpr(left->GetFloat() sym right->GetInt());  \
        return true;                                              \
    }

    switch (op)
    {
    case Tag::plus: handle_calc(+); break;
    case Tag::minus: handle_calc(-); break;
    case Tag::asterisk: handle_calc(*); break;
    case Tag::slash: handle_calc(/); break;
    case Tag::_and: handle_int(&); break;
    case Tag::incl_or: handle_int(|); break;
    case Tag::logical_and: handle_calc(&&); break;
    case Tag::logical_or: handle_calc(||); break;
    case Tag::lshift: handle_int(<<); break;
    case Tag::rshift: handle_int(>>); break;
    case Tag::lessthan: handle_calc(<); break;
    case Tag::greathan: handle_calc(>); break;
    case Tag::lessequal: handle_calc(<=); break;
    case Tag::greatequal: handle_calc(>=); break;
    case Tag::cap: handle_int(^); break;
    case Tag::equal: handle_calc(==); break;
    case Tag::notequal: handle_calc(!=); break;
    default: return false;
    }

    return false;

#undef both_int
#undef both_float
#undef int_float
#undef float_int
#undef handle_int
#undef handle_calc
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

