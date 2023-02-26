#include "visitast/Evaluator.h"
#include "IR/IROperand.h"


bool Evaluator::IsComparsionTag(Tag op)
{
    return op == Tag::lessthan || op == Tag::lessequal ||
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

template <typename RET, typename NUM>
RET Evaluator::Calc(Tag op, NUM num)
{
    switch (op)
    {
    case Tag::plus: return (RET)num;
    case Tag::minus: return (RET)(-num);
    case Tag::exclamation: return (RET)(!num);
    case Tag::tilde: return (RET)(~num);
    default: return 0;
    }
}


#define to_int(op) static_cast<const IntConst*>(op)
#define to_float(op) static_cast<const FloatConst*>(op)

const IROperand* Evaluator::EvalBinary(Function* func, Tag op, const IROperand* lhs, const IROperand* rhs)
{
    if (lhs->IsFloatConst() && rhs->IsFloatConst())
    {
        auto left = to_float(lhs)->Val();
        auto right = to_float(rhs)->Val();

        if (IsComparsionTag(op))
            return IntConst::CreateIntConst(
                func,
                Calc<unsigned long, double, double>(op, left, right),
                IntType::GetInt32(true));

        double result = Calc<double, double, double>(op, left, right);
        auto ty = lhs->Type()->operator>(*rhs->Type()) ? lhs->Type() : rhs->Type();
        return FloatConst::CreateFloatConst(func, result, ty->ToFloatPoint());
    }
    else if (lhs->IsFloatConst() && rhs->IsIntConst())
    {
        auto left = to_float(lhs)->Val();
        auto right = to_int(rhs)->Val();

        double result;
        if (rhs->Type()->ToInteger()->IsSigned())
            result = Calc<double, double, double>(op, left, (long)right);
        else result = Calc<double, double, double>(op, left, right);

        if (IsComparsionTag(op))
            return IntConst::CreateIntConst(func, (unsigned long)result, IntType::GetInt32(true));
        return FloatConst::CreateFloatConst(func, result, lhs->Type()->ToFloatPoint());
    }
    else if (lhs->IsIntConst() && rhs->IsFloatConst())
    {
        auto left = to_int(lhs)->Val();
        auto right = to_float(rhs)->Val();
        double result;
        if (lhs->Type()->ToInteger()->IsSigned())
            result = Calc<double, double, double>(op, (long)left, right);
        else result = Calc<double, double, double>(op, left, right);

        if (IsComparsionTag(op))
            return IntConst::CreateIntConst(func, (unsigned long)result, IntType::GetInt32(true));
        return FloatConst::CreateFloatConst(func, result, rhs->Type()->ToFloatPoint());
    }
    else
    {
        auto left = to_int(lhs)->Val();
        auto right = to_int(rhs)->Val();

        bool lsigned = lhs->Type()->ToInteger()->IsSigned();
        bool rsigned = rhs->Type()->ToInteger()->IsSigned();

        const IntType* ty = nullptr;
        if (IsComparsionTag(op))
            ty = IntType::GetInt32(true);
        else if (lhs->Type()->operator>(*rhs->Type()))
            ty = lhs->Type()->ToInteger();
        else if (lhs->Type()->operator<(*rhs->Type()))
            ty = rhs->Type()->ToInteger();
        else if ((!lsigned && !rsigned) || (lsigned && rsigned))
            ty = lhs->Type()->ToInteger();
        else if (lsigned || rsigned)
            ty = lsigned ? rhs->Type()->ToInteger() : lhs->Type()->ToInteger();

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

        return IntConst::CreateIntConst(func, result, ty);
    }
}

const IROperand* Evaluator::EvalUnary(Function* func, Tag op, const IROperand* num)
{
    if (num->IsIntConst())
    {
        unsigned long result = 0;
        if (num->Type()->ToInteger()->IsSigned())
            result = Calc<long, unsigned long>(op, to_int(num)->Val());
        else result = Calc<long, long>(op, to_int(num)->Val());

        return IntConst::CreateIntConst(func, result, num->Type()->ToInteger());
    }
    else
    {
        double result = Calc<double, double>(op, to_float(num)->Val());
        return FloatConst::CreateFloatConst(func, result, num->Type()->ToFloatPoint());
    }
}

#undef to_int
#undef to_float
