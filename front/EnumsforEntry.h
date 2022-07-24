#ifndef _ENUMS_FOR_ENTRY_H_
#define _ENUMS_FOR_ENTRY_H_

enum class typespec // type specifier
{
    int8,
    int16,
    int32,
    int64,
    uint8,
    uint16,
    uint32,
    uint64,
    float32,
    float64,
    _void, // void
};

enum class typequal // type qualifier
{
    _const,
    _restrict,
    _volatile,
    _atomic
};

#endif // _ENUMS_FOR_ENTRY_H_
