#ifndef _NODE_H_
#define _NODE_H_

#include "../utilities/IR.h"
#include <string>

enum class Tag
{
    _typedef, _extern, _static, _thread_local,
    
    _auto, _register, _void, _char, _short, _int,
    _long, _float, _double, _signed, _unsigned,
    _bool, _complex, _imaginary, atomic_type_specifier,
    struct_or_union_specifier, enum_specifier, typedef_name,

    _const, _restrict, _volatile, _atomic,

    arrow, inc, dec, dot,
    _and, star, plus, minus, tilde, exclamation, 
    slash, percent, lshift, rshift, greathan,
    lessthan, greatequal, lessequal, equal,
    notequal, cap, 
    _sizeof, _alignof,

    mul_assign, div_assign, mod_assign, add_assign,
    sub_assign, left_assign, right_assign, and_assign,
    xor_assign, or_assign
};

class Node
{
public:
    virtual ~Node() = default;
};

#endif // _NODE_H_
