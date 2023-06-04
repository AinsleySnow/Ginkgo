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


const IROperand* Evaluator::EvalBinary(Pool<IROperand>* pool, Tag op, const IROperand* lhs, const IROperand* rhs)
{
    if (lhs->Is<FloatConst>() && rhs->Is<FloatConst>())
    {
        auto left = lhs->As<FloatConst>()->Val();
        auto right = rhs->As<FloatConst>()->Val();

        if (IsLogicalTag(op))
            return IntConst::CreateIntConst(
                pool,
                Calc<double, double, double>(op, left, right),
                IntType::GetInt32(true));

        double result = Calc<double, double, double>(op, left, right);
        auto ty = lhs->Type()->operator>(*rhs->Type()) ? lhs->Type() : rhs->Type();
        return FloatConst::CreateFloatConst(pool, result, ty->As<FloatType>());
    }
    else if (lhs->Is<FloatConst>() && rhs->Is<IntConst>())
    {
        auto left = lhs->As<FloatConst>()->Val();
        auto right = rhs->As<IntConst>()->Val();

        double result;
        if (rhs->Type()->As<IntType>()->IsSigned())
            result = Calc<double, long>(op, left, (long)right);
        else result = Calc<double, unsigned long>(op, left, right);

        if (IsLogicalTag(op))
            return IntConst::CreateIntConst(pool, (unsigned long)result, IntType::GetInt32(true));
        return FloatConst::CreateFloatConst(pool, result, lhs->Type()->As<FloatType>());
    }
    else if (lhs->Is<IntConst>() && rhs->Is<FloatConst>())
    {
        auto left = lhs->As<IntConst>()->Val();
        auto right = rhs->As<FloatConst>()->Val();
        double result;
        if (lhs->Type()->As<IntType>()->IsSigned())
            result = Calc<long, double>(op, (long)left, right);
        else result = Calc<unsigned long, double>(op, left, right);

        if (IsLogicalTag(op))
            return IntConst::CreateIntConst(pool, (unsigned long)result, IntType::GetInt32(true));
        return FloatConst::CreateFloatConst(pool, result, rhs->Type()->As<FloatType>());
    }
    else
    {
        auto left = lhs->As<IntConst>()->Val();
        auto right = rhs->As<IntConst>()->Val();

        if (op == Tag::lshift || op == Tag::rshift)
            right %= lhs->Type()->Size() * 8;

        bool lsigned = lhs->Type()->As<IntType>()->IsSigned();
        bool rsigned = rhs->Type()->As<IntType>()->IsSigned();

        const IntType* ty = nullptr;
        if (IsLogicalTag(op))
            ty = IntType::GetInt32(true);
        else if (lhs->Type()->operator>(*rhs->Type()))
            ty = lhs->Type()->As<IntType>();
        else if (lhs->Type()->operator<(*rhs->Type()))
            ty = rhs->Type()->As<IntType>();
        else if ((!lsigned && !rsigned) || (lsigned && rsigned))
            ty = lhs->Type()->As<IntType>();
        else if (lsigned || rsigned)
            ty = lsigned ? rhs->Type()->As<IntType>() : lhs->Type()->As<IntType>();

        unsigned long result = 0;
        if (lsigned && rsigned)
            result = Calc<unsigned long, long, long>(op, left, right);
        else if (!lsigned && rsigned)
        {
            if (lhs->Type()->operator<(*rhs->Type()))
                result = Calc<unsigned long, long, long>(op, left, right);
            else result = Calc<
                unsigned long, unsigned long, unsigned long>(op, left, right);
        }
        else if (lsigned && !rsigned)
        {
            if (lhs->Type()->operator>(*rhs->Type()))
                result = Calc<unsigned long, long, long>(op, left, right);
            else result = Calc<
                unsigned long, unsigned long, unsigned long>(op, left, right);
        }
        else if (!lsigned && !rsigned)
            result = Calc<unsigned long, unsigned long, unsigned long>(op, left, right);

        // Otherwise, things like (1 << 31) >> 1 will yield 1 but not -1.
        if (op == Tag::rshift &&
            lhs->Type()->As<IntType>()->IsSigned() &&
            (left & (1 << (lhs->Type()->Size() * 8 - 1))))
            result |= (unsigned long)(-1ll >> (64 - lhs->Type()->Size() * 8 + right - 1));

        return IntConst::CreateIntConst(pool, result, ty);
    }
}

const IROperand* Evaluator::EvalUnary(Pool<IROperand>* pool, Tag op, const IROperand* num)
{
    if (num->Is<IntConst>())
    {
        unsigned long result = 0;
        if (num->Type()->As<IntType>()->IsSigned())
            result = Calc<long, unsigned long>(op, num->As<IntConst>()->Val());
        else result = Calc<long, long>(op, num->As<IntConst>()->Val());

        return IntConst::CreateIntConst(pool, result, num->Type()->As<IntType>());
    }
    else
    {
        double result = Calc<double, double>(op, num->As<FloatConst>()->Val());
        if (op == Tag::exclamation)
            return IntConst::CreateIntConst(pool, result, IntType::GetInt32(true));
        else return FloatConst::CreateFloatConst(pool, result, num->Type()->As<FloatType>());
    }
}

#undef to_int
#undef to_float
