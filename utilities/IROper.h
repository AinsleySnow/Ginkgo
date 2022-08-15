#ifndef _IROPER_H_
#define _IROPER_H_

enum class IROper
{
    assign,
    add, subtract,
    multiple, divide, mod,
    inc, dec,
    getaddr, dereference, 
    positive, negative, 
    bitwisenot, bitwiseand, bitwiseor, bitwisexor,
    logicalnot, logicaland, logicalor,
    lshift, rshift, 
    less, great, lessequal, greatequal,
    notequal, equal,
    getsize, getalign,

    lable,
    tag, jmp, jmptrue, jmpfalse
};

#endif // _IROPER_H_
