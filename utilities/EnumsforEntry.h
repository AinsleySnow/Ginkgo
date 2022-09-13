#ifndef _ENUMS_FOR_ENTRY_H_
#define _ENUMS_FOR_ENTRY_H_

enum class TypeQual // type qualifier
{
    _const,
    _restrict,
    _volatile,
    _atomic
};

enum class StorageSpec
{
    _extern = 1,
    _static = 2,
    _thread_local = 4,
    _auto = 8,
    _register = 16
};

#endif // _ENUMS_FOR_ENTRY_H_
