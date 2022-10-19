#ifndef _ERROR_H_
#define _ERROR_H_


enum class ErrorId
{
    invaildtypespec,
    needlval,
    uncompletetype,
    operatormisuse,
    multistorage,
    cannotconcatenate
};

void Error(ErrorId id, ...);

#endif // _ERROR_H_
