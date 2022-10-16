#ifndef _ERROR_H_
#define _ERROR_H_

void Error(ErrorId id, ...);

enum class ErrorId
{
    invaildtypespec,
    needlval,
    uncompletetype,
    operatormisuse,
    multistorage,
    
};

#endif // _ERROR_H_
