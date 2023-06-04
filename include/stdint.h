#ifndef __GINKGO_STDINT_H
#define __GINKGO_STDINT_H

#define __STDC_VERSION_STDINT_H__ 202311L

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

typedef char int_least8_t;
typedef short int int_least16_t;
typedef int int_least32_t;
typedef long long int int_least64_t;

typedef unsigned char uint_least8_t;
typedef unsigned short int uint_least16_t;
typedef unsigned int uint_least32_t;
typedef unsigned long long int uint_least64_t;


// better be consistent with GNU's header.
typedef char int_fast8_t;
typedef long int_fast16_t;
typedef long int_fast32_t;
typedef long int_fast64_t;

typedef unsigned char uint_fast8_t;
typedef unsigned long uint_fast16_t;
typedef unsigned long uint_fast32_t;
typedef unsigned long uint_fast64_t;


typedef long intptr_t;
typedef unsigned long uintptr_t;
typedef long intmax_t;
typedef unsigned long uintmax_t;


#define INT8_WIDTH 8
#define INT16_WIDTH 16
#define INT32_WIDTH 32
#define INT64_WIDTH 64

#define UINT8_WIDTH 8
#define UINT16_WIDTH 16
#define UINT32_WIDTH 32
#define UINT64_WIDTH 64

#define INT_LEAST8_WIDTH 8
#define INT_LEAST16_WIDTH 16
#define INT_LEAST32_WIDTH 32
#define INT_LEAST64_WIDTH 64

#define UINT_LEAST8_WIDTH 8
#define UINT_LEAST16_WIDTH 16
#define UINT_LEAST32_WIDTH 32
#define UINT_LEAST64_WIDTH 64

#define INT_FAST8_WIDTH 8
#define INT_FAST16_WIDTH 64
#define INT_FAST32_WIDTH 64
#define INT_FAST64_WIDTH 64

#define UINT_FAST8_WIDTH 8
#define UINT_FAST16_WIDTH 64
#define UINT_FAST32_WIDTH 64
#define UINT_FAST64_WIDTH 64

#define INTPTR_WIDTH 64
#define UINTPTR_WIDTH 64

#define INTMAX_WIDTH 64
#define INTMAX_WIDTH 64

#define PTRDIFF_WIDTH 64
#define SIG_ATOMIC_WIDTH 32

#define SIZE_WIDTH 64
#define WCHAR_WIDTH 32
#define WINT_WIDTH 32

#define INTMAX_C(value) value ## LL
#define UINTMAX_C(value) value ## ULL


#include <limits.h>

#define INT8_MIN CHAR_MIN
#define INT8_MAX CHAR_MAX
#define INT16_MIN SHRT_MIN
#define INT16_MAX SHRT_MAX
#define INT32_MIN INT_MIN
#define INT32_MAX INT_MAX
#define INT64_MIN LLONG_MIN
#define INT64_MAX LLONG_MAX

#define UINT8_MAX UCHAR_MAX
#define UINT16_MAX USHRT_MAX
#define UINT32_MAX UINT_MAX
#define UINT64_MAX ULLONG_MAX


#define INT_LEAST8_MIN CHAR_MIN
#define INT_LEAST8_MAX CHAR_MAX
#define INT_LEAST16_MIN SHRT_MIN
#define INT_LEAST16_MAX SHRT_MAX
#define INT_LEAST32_MIN INT_MIN
#define INT_LEAST32_MAX INT_MAX
#define INT_LEAST64_MIN LLONG_MIN
#define INT_LEAST64_MAX LLONG_MAX

#define UINT_LEAST8_MAX UCHAR_MAX
#define UINT_LEAST16_MAX USHRT_MAX
#define UINT_LEAST32_MAX UINT_MAX
#define UINT_LEAST64_MAX ULLONG_MAX


#define INT_FAST8_MIN CHAR_MIN
#define INT_FAST8_MAX CHAR_MAX
#define INT_FAST16_MIN LLONG_MIN
#define INT_FAST16_MAX LLONG_MAX
#define INT_FAST32_MIN LLONG_MIN
#define INT_FAST32_MAX LLONG_MAX
#define INT_FAST64_MIN LLONG_MIN
#define INT_FAST64_MAX LLONG_MAX

#define UINT_FAST8_MAX CHAR_MAX
#define UINT_FAST16_MAX LLONG_MAX
#define UINT_FAST32_MAX LLONG_MAX
#define UINT_FAST64_MAX LLONG_MAX


#define INTPTR_MIN LLONG_MIN
#define INTPTR_MAX LLONG_MAX
#define UINTPTR_MAX ULLONG_MAX

#define INTMAX_MIN LLONG_MIN
#define INTMAX_MAX LLONG_MAX
#define UINTMAX_MAX ULLONG_MAX

#define PTRDIFF_MIN LLONG_MIN
#define PTRDIFF_MAX LLONG_MAX

#define SIG_ATOMIC_MIN INT_MIN
#define SIG_ATOMIC_MAX INT_MAX

#define SIZE_MAX ULLONG_MAX
#define WCHAR_MIN INT_MIN
#define WCHAR_MAX INT_MAX

#define WINT_MIN 0
#define WINT_MAX UINT_MAX

#define INT8_C(value)   value
#define INT16_C(value)  value
#define INT32_C(value)  value
#define INT64_C(value)  value ## LL

#define UINT8_C(value)   value
#define UINT16_C(value)  value
#define UINT32_C(value)  value
#define UINT64_C(value)  value ## ULL


#endif // __GINKGO_STDINT_H
