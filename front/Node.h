#ifndef _NODE_H_
#define _NODE_H_

#include "../IR/IR.h"
#include <string>

enum class Tag
{
    declaration,
    declaration_specifiers,
    declarator, 
    direct_declarator,
    identifier,
    initializer, 
    init_declarator, 
    init_declarator_list,
    static_assert_declaration,

    _typedef, _extern, _static, _thread_local,
    
    _auto, _register, _void, _char, _short, _int,
    _long, _float, _double, _signed, _unsigned,
    _bool, _complex, _imaginary, atomic_type_specifier,
    struct_or_union_specifier, enum_specifier, typedef_name,

    _const, _restrict, _volatile, _atomic
};

class Node
{
private:
    Tag tag;
    std::string literal{};

public:
    Node(Tag);
    Node(Tag, const std::string&);

    Tag GetTag() const;
    std::string GetLiteral() const;

    virtual std::string ToString() const;
    virtual IR Gen();
};

#endif // _NODE_H_
