#include "Constant.h"
#include "ArithmType.h"
#include "messages/Error.h"

bool Constant::DoCalc(Tag op, const Constant& num, Constant& output)
{
    switch (op)
    {
    case Tag::dec: 
    }
}

bool Constant::DoCalc(Tag op, const Constant& left, const Constant& right, Constant& output)
{
#define both_float (left.GetType())->IsFloat() && (right.GetType())->IsFloat()
#define both_int (left.GetType())->IsInteger() && (right.GetType())->IsInteger()
#define int_float (left.GetType())->IsInteger() && (right.GetType())->IsFloat()
#define float_int (left.GetType())->IsFloat() && (right.GetType())->IsInteger()
#define handle_int(sym)                                         \
    if (both_int)                                               \
    {                                                           \
        output = Constant(left.GetInt() sym right.GetInt());    \
        return true;                                            \
    }                                                           \
    else                                                        \
    {                                                           \
        Error(0);                                               \
        return false;                                           \
    }

#define handle_calc(sym)                                        \
    if (both_float)                                             \
    {                                                           \
        output = Constant(left.GetFloat() sym right.GetFloat());\
        return true;                                            \
    }                                                           \
    else if (both_int)                                          \
    {                                                           \
        output = Constant(left.GetInt() sym right.GetInt());    \
        return true;                                            \
    }                                                           \
    else if (int_float)                                         \
    {                                                           \
        output = Constant(left.GetInt() sym right.GetFloat());  \
        return true;                                            \
    }                                                           \
    else if (float_int)                                         \
    {                                                           \
        output = Constant(left.GetFloat() sym right.GetInt());  \
        return true;                                            \
    }


    switch (op)
    {
    case Tag::plus: handle_calc(+); break;
    case Tag::minus: handle_calc(-); break;
    case Tag::star: handle_calc(*); break;
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
    }

    Error(1);
    return false;

#undef both_int
#undef both_float
#undef int_float
#undef float_int
#undef handle_int
#undef handle_calc
}