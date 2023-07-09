#ifndef _TAG_H_
#define _TAG_H_

enum class Tag
{
    _char = 1, _short = 2, _int = 4, _long = 8,
    _longlong = 24, _float = 32, _double = 64,
    _unsigned = 128, _signed = 256, _bool = 512,
    _void = 1024, _enum = 2048, _struct = 4096,
    _union = 8192, _typedef = 16384, _atomic = 32768,

    // _typedef and _atomic can be used in two
    // different scenarios.
    // first: in type_specifier meaning that
    // the type is defined by typedef; 
    // second: in storage specifier,
    // just a qualifier of a variable
    _extern, _static, _thread_local,

    _auto, _register, _complex, _imaginary,
    
    _const, _restrict, _volatile,

    _inline, _noreturn,

    inc, dec, postfix_inc, postfix_dec,
    arrow,  dot,
    _and, _or, _xor, asterisk, plus, minus, tilde, exclamation, 
    slash, percent, lshift, rshift, greathan,
    lessthan, greatequal, lessequal, equal, notequal,
    logical_and, logical_or,
    _sizeof, _alignof, _alignas,
    left, right,

    assign, mul_assign, div_assign, mod_assign, add_assign,
    sub_assign, left_assign, right_assign, and_assign,
    xor_assign, or_assign,

    _case, _default, _else, _switch, _continue, _do,
    _for, _if, _while, _goto, _return, _break
};

#endif // _TAG_H_
