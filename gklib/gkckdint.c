#include <stdbool.h>

#define GK_CKD(usage, sign, size) __Ginkgo_ckd_ ## usage ## _ ## sign ## size
#define WITH_TYPE(type) (type* result, type lhs, type rhs)

// Both type2 and type3 shall be any integer type other than "plain"
// char, bool, a bit-precise integer type, or an enumerated type, and
// they need not be the same. *result shall be a modifiable lvalue
// of any integer type other than "plain" char, bool, a bit-precise
// integer type, or an enumerated type. (7.20.1[3])
// But I would like to remove the restriction here and allow ckd_xxx
// to accept any integer type, such as char, bool, typedefed types,
// and enumerated types. CodeChk (will) provide a warning if the user
// writes something that is excessively problematic.

#define DO_ARITHM(operator)     \
    long res = 0;               \
    long left = lhs;            \
    long right = rhs;           \
    res = left operator right

#define CHECK(num, offset) (!!((num) & (1 << (offset))))
#define SIGNED(num) (!!(((typeof(num))1 << ((sizeof(typeof(num))) * 8) - 1) & (num)))

#define EVAL_UNSIGNED(operator, type, size)     \
    DO_ARITHM(operator);                        \
    *result = (type)res;                        \
    long max = 1 << ((size) - 1);               \
    return res > max

#define EVAL_SIGNED(operator, type, size)       \
    DO_ARITHM(operator);                        \
    *result = (type)res;                        \
    long max = 1 << ((size) - 2);               \
    long min = ((long)1 << 63) >> (64 - size);  \
    return res > max || res < min


bool GK_CKD(add, u, 8) WITH_TYPE(char)   { EVAL_UNSIGNED(+, char, 8); }
bool GK_CKD(add, s, 8) WITH_TYPE(char)   { EVAL_SIGNED(+, char, 8); }
bool GK_CKD(add, u, 16) WITH_TYPE(short) { EVAL_UNSIGNED(+, short, 16); }
bool GK_CKD(add, s, 16) WITH_TYPE(short) { EVAL_SIGNED(+, short, 16); }
bool GK_CKD(add, u, 32) WITH_TYPE(int)   { EVAL_UNSIGNED(+, int, 32); }
bool GK_CKD(add, s, 32) WITH_TYPE(int)   { EVAL_SIGNED(+, int, 32); }

bool GK_CKD(add, u, 64) WITH_TYPE(unsigned long)
{
    DO_ARITHM(+);
    char carry = 0;
    asm ("setc %0" : "=r" (carry));
    *result = (unsigned long)res;
    return carry;
}

bool GK_CKD(add, s, 64) WITH_TYPE(long)
{
    DO_ARITHM(+);
    char offset = 0;
    asm ("seto %0" : "=r" (offset));
    *result = (long)res;
    return (offset & !(SIGNED(lhs) ^ SIGNED(rhs)));
}


bool GK_CKD(sub, u, 8) WITH_TYPE(char)   { EVAL_UNSIGNED(-, char, 8); }
bool GK_CKD(sub, s, 8) WITH_TYPE(char)   { EVAL_SIGNED(-, char, 8); }
bool GK_CKD(sub, u, 16) WITH_TYPE(short) { EVAL_UNSIGNED(-, short, 16); }
bool GK_CKD(sub, s, 16) WITH_TYPE(short) { EVAL_SIGNED(-, short, 16); }
bool GK_CKD(sub, u, 32) WITH_TYPE(int)   { EVAL_UNSIGNED(-, int, 32); }
bool GK_CKD(sub, s, 32) WITH_TYPE(int)   { EVAL_SIGNED(-, int, 32); }

// There is no need to consisder the scenario where the second parameter
// is negative. The built-in magic will recognize the case, convert the
// parameter to a positive value and call ckd_add instead.

bool GK_CKD(sub, u, 64) WITH_TYPE(unsigned long)
{
    DO_ARITHM(-);
    char carry = 0;
    asm ("setc %0" : "=r" (carry));
    *result = (long)res;
    return carry;
}

bool GK_CKD(sub, s, 64) WITH_TYPE(long)
{
    DO_ARITHM(-);
    char offset = 0;
    asm ("seto %0" : "=r" (offset));
    *result = (long)res;
    return offset & !(SIGNED(lhs) ^ SIGNED(rhs));
}


#define DO_SIGNED_MULTIPLY      \
    long res = 0;               \
    long left = (long)lhs;      \
    long right = (long)rhs;     \
    res = left * right

#define DO_UNSIGNED_MULTIPLY                    \
    unsigned long res = 0;                      \
    unsigned long left = (unsigned long)lhs;    \
    unsigned long right = (unsigned long)rhs;   \
    res = left * right

#define EVAL_SIGNED_MULTIPLY(type, size)        \
    DO_SIGNED_MULTIPLY;                         \
    *result = (type)res;                        \
    long max = 1 << ((size) - 2);               \
    long min = ((long)1 << 63) >> (64 - size);  \
    return res > max || res < min

#define EVAL_UNSIGNED_MULTIPLY(type, size)      \
    DO_UNSIGNED_MULTIPLY;                       \
    *result = (type)res;                        \
    unsigned long max = 1 << ((size) - 1);      \
    return res > max


bool GK_CKD(mul, u, 8) WITH_TYPE(unsigned char)     { EVAL_UNSIGNED_MULTIPLY(unsigned char, 8); }
bool GK_CKD(mul, s, 8) WITH_TYPE(char)              { EVAL_SIGNED_MULTIPLY(char, 8); }
bool GK_CKD(mul, u, 16) WITH_TYPE(unsigned short)   { EVAL_UNSIGNED_MULTIPLY(short, 16); }
bool GK_CKD(mul, s, 16) WITH_TYPE(short)            { EVAL_SIGNED_MULTIPLY(short, 16); }
bool GK_CKD(mul, u, 32) WITH_TYPE(int)              { EVAL_UNSIGNED_MULTIPLY(int, 32); }
bool GK_CKD(mul, s, 32) WITH_TYPE(int)              { EVAL_SIGNED_MULTIPLY(int, 32); }

// There is no need to consisder the scenario where either the parameter
// is negative in the unsigned version. The built-in magic will recognize
// the case, convert all the parameters to positive values, and attach the
// sign to the result after the calculation is done.

bool GK_CKD(mul, u, 64) WITH_TYPE(unsigned long)
{
    *result = lhs * rhs;
    char carry = 0;
    asm ("setc %0" : "=r" (carry));
    return carry;
}

bool GK_CKD(mul, s, 64) WITH_TYPE(long)
{
    *result = lhs * rhs;
    char carry = 0;
    char overflow = 0;
    asm ("setc %0" : "=r" (carry));
    asm ("seto %0" : "=r" (overflow));
    return ((overflow | carry) & (!SIGNED(lhs) & !SIGNED(rhs)));
}
