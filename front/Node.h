#ifndef _NODE_H_
#define _NODE_H_

#include "../IR/IR.h"
#include <string>

enum class Tag
{
    _typedef, _extern, _static, _thread_local,
    
    _auto, _register, _void, _char, _short, _int,
    _long, _float, _double, _signed, _unsigned,
    _bool, _complex, _imaginary, atomic_type_specifier,
    struct_or_union_specifier, enum_specifier, typedef_name,

    _const, _restrict, _volatile, _atomic
};

class Node
{
public:
    virtual ~Node() = default;

    virtual std::string ToString() const;
    virtual IR Generate() const;
};

#endif // _NODE_H_
