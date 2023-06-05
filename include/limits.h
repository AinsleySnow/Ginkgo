#ifndef __GINKGO_LIMITS_H
#define __GINKGO_LIMITS_H

#define BOOL_WIDTH      1
#define BOOL_MAX        1

#define CHAR_BIT        8
#define CHAR_WIDTH      8
#define CHAR_MIN        0
#define CHAR_MAX        255
#define SCHAR_WIDTH     8
#define UCHAR_WIDTH     8
#define SCHAR_MIN       (-128)
#define SCHAR_MAX       127
#define UCHAR_MAX       255


#define SHRT_WIDTH      16
#define USHRT_WIDTH     16
#define SHRT_MIN        (-32768)
#define SHRT_MAX        32767
#define USHRT_MAX       65535

#define INT_WIDTH       32
#define UINT_WIDTH      32
#define INT_MIN         (-2147483648)
#define INT_MAX         2147483647
#define UINT_MAX        4294967295U

#define LONG_WIDTH      64
#define ULONG_WIDTH     64
#define LONG_MIN        (-9223372036854775808L)
#define LONG_MAX        9223372036854775807L
#define ULONG_MAX       18446744073709551615UL

#define LLONG_WIDTH     64
#define ULLONG_WIDTH    64
#define LLONG_MIN       (-9223372036854775808LL)
#define LLONG_MAX       9223372036854775807LL
#define ULLONG_MAX      18446744073709551615ULL

// keep consistant with GNU's header.
#define MB_LEN_MAX      16

#endif // __GINKGO_LIMITS_H
