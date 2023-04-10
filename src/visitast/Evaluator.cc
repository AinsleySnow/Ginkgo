#include "visitast/Evaluator.h"
#include "IR/IROperand.h"


bool Evaluator::IsLogicalTag(Tag op)
{
    return op == Tag::logical_and || op == Tag::logical_or ||
        op == Tag::lessthan || op == Tag::lessequal ||
        op == Tag::greathan || op == Tag::greatequal ||
        op == Tag::equal || op == Tag::notequal;
}


template <typename RET, typename LHS, typename RHS>
RET Evaluator::Calc(Tag op, LHS l, RHS r)
{
    switch (op)
    {
    case Tag::plus: return (RET)(l + r);
    case Tag::minus: return (RET)(l - r);
    case Tag::asterisk: return (RET)(l * r);
    case Tag::slash: return (RET)(l / r);
    case Tag::percent: return (RET)(l % r);
    case Tag::_and: return (RET)(l & r);
    case Tag::_or: return (RET)(l | r);
    case Tag::logical_and: return (RET)(l && r);
    case Tag::logical_or: return (RET)(l || r);
    case Tag::lshift: return (RET)(l << r);
    case Tag::rshift: return (RET)(l >> r);
    case Tag::lessthan: return (RET)(l < r);
    case Tag::greathan: return (RET)(l > r);
    case Tag::lessequal: return (RET)(l <= r);
    case Tag::greatequal: return (RET)(l >= r);
    case Tag::_xor: return (RET)(l ^ r);
    case Tag::equal: return (RET)(l == r);
    case Tag::notequal: return (RET)(l != r);
    default: return 0;
    }
}


#define switch_body                     \
switch (op)                             \
{                                       \
case Tag::plus: return l + r;           \
case Tag::minus: return l - r;          \
case Tag::asterisk: return l * r;       \
case Tag::slash: return l / r;          \
case Tag::logical_and: return l && r;   \
case Tag::logical_or: return l || r;    \
case Tag::lessthan: return l < r;       \
case Tag::greathan: return l > r;       \
case Tag::lessequal: return l <= r;     \
case Tag::greatequal: return l >= r;    \
case Tag::equal: return l == r;         \
case Tag::notequal: return l != r;      \
default: return 0;                      \
}

template <>
double Evaluator::Calc<double, double, double>(Tag op, double l, double r)
{
    switch_body;
}

template <typename LHS, typename RHS>
double Evaluator::Calc(Tag op, LHS l, RHS r)
{
    switch_body;
}

#undef switch_body


template <typename RET, typename NUM>
RET Evaluator::Calc(Tag op, NUM num)
{
    switch (op)
    {
    case Tag::plus: return (RET)num;
    case Tag::minus: return (RET)(-num);
    case Tag::exclamation: return (RET)(!num);
    case Tag::tilde: return (RET)(~(unsigned long)num);
    default: return 0;
    }
}


#define to_int(op) static_cast<const IntConst*>(op)
#define to_float(op) static_cast<const FloatConst*>(op)

const IROperand* Evaluator::EvalBinary(Pool<IROperand>* pool, Tag op, const IROperand* lhs, const IROperand* rhs)
{
    if (lhs->IsFloatConst() && rhs->IsFloatConst())
    {
        auto left = to_float(lhs)->Val();
        auto right = to_float(rhs)->Val();

        if (IsLogicalTag(op))
            return IntConst::CreateIntConst(
                pool,
                Calc<double, double, double>(op, left, right),
                IntType::GetInt32(true));

        double result = Calc<double, double, double>(op, left, right);
        auto ty = lhs->Type()->operator>(*rhs->Type()) ? lhs->Type() : rhs->Type();
        return FloatConst::CreateFloatConst(pool, result, ty->ToFloatPoint());
    }
    else if (lhs->IsFloatConst() && rhs->IsIntConst())
    {
        auto left = to_float(lhs)->Val();
        auto right = to_int(rhs)->Val();

        double result;
        if (rhs->Type()->ToInteger()->IsSigned())
            result = Calc<double, long>(op, left, (long)right);
        else result = Calc<double, unsigned long>(op, left, right);

        if (IsLogicalTag(op))
            return IntConst::CreateIntConst(pool, (unsigned long)result, IntType::GetInt32(true));
        return FloatConst::CreateFloatConst(pool, result, lhs->Type()->ToFloatPoint());
    }
    else if (lhs->IsIntConst() && rhs->IsFloatConst())
    {
        auto left = to_int(lhs)->Val();
        auto right = to_float(rhs)->Val();
        double result;
        if (lhs->Type()->ToInteger()->IsSigned())
            result = Calc<long, double>(op, (long)left, right);
        else result = Calc<unsigned long, double>(op, left, right);

        if (IsLogicalTag(op))
            return IntConst::CreateIntConst(pool, (unsigned long)result, IntType::GetInt32(true));
        return FloatConst::CreateFloatConst(pool, result, rhs->Type()->ToFloatPoint());
    }
    else
    {
        auto left = to_int(lhs)->Val();
        auto right = to_int(rhs)->Val();

        if (op == Tag::lshift || op == Tag::rshift)
            right %= lhs->Type()->Size() * 8;

        bool lsigned = lhs->Type()->ToInteger()->IsSigned();
        bool rsigned = rhs->Type()->ToInteger()->IsSigned();

        const IntType* ty = nullptr;
        if (IsLogicalTag(op))
            ty = IntType::GetInt32(true);
        else if (lhs->Type()->operator>(*rhs->Type()))
            ty = lhs->Type()->ToInteger();
        else if (lhs->Type()->operator<(*rhs->Type()))
            ty = rhs->Type()->ToInteger();
        else if ((!lsigned && !rsigned) || (lsigned && rsigned))
            ty = lhs->Type()->ToInteger();
        else if (lsigned || rsigned)
            ty = lsigned ? rhs->Type()->ToInteger() : lhs->Type()->ToInteger();

        unsigned long result = Calc<unsigned long, long, long>(op, left, right);

        if (op == Tag::rshift &&
            lhs->Type()->ToInteger()->IsSigned() &&
            (left & (1 << (lhs->Type()->Size() * 8 - 1))))
            result |= (unsigned long)(-1ll >> (64 - lhs->Type()->Size() * 8 + right - 1));

        return IntConst::CreateIntConst(pool, result, ty);
    }
}

const IROperand* Evaluator::EvalUnary(Pool<IROperand>* pool, Tag op, const IROperand* num)
{
    if (num->IsIntConst())
    {
        unsigned long result = 0;
        if (num->Type()->ToInteger()->IsSigned())
            result = Calc<long, unsigned long>(op, to_int(num)->Val());
        else result = Calc<long, long>(op, to_int(num)->Val());

        return IntConst::CreateIntConst(pool, result, num->Type()->ToInteger());
    }
    else
    {
        double result = Calc<double, double>(op, to_float(num)->Val());
        if (op == Tag::exclamation)
            return IntConst::CreateIntConst(pool, result, IntType::GetInt32(true));
        else return FloatConst::CreateFloatConst(pool, result, num->Type()->ToFloatPoint());
    }
}

#undef to_int
#undef to_float
