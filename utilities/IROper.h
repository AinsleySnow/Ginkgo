#ifndef _IROPER_H_
#define _IROPER_H_

enum class IROper
{
    assign,
    plus, minus,
    multiple, divide, mod,
    inc, dec,
    getaddr, dereference, positive, negative, bitwisenot, logicalnot,
    getsize, getalign
};

#endif // _IROPER_H_
