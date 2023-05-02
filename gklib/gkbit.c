// implementation of functions in stdbit.h of C23.
// It's a header file newly introduced in C23, and glibc has not
// implemented these functions when Ginkgo was building (2023 spring).
// So I implemented my own version.
// I didn't pay much attentation to compatibility, so the underlying
// instructions may not be supported by your CPU. However, on my
// own processor (Intel Core i5-1035G1), they work well.
#include <immintrin.h>

int stdc_leading_zerosuc(unsigned char v) { return _lzcnt_u32(v) - 24; }
int stdc_leading_zerosus(unsigned short v) { return _lzcnt_u32(v) - 16; }
int stdc_leading_zerosui(unsigned int v) { return _lzcnt_u32(v); }
int stdc_leading_zerosul(unsigned long v) { return _lzcnt_u64(v); }
int stdc_leading_zerosull(unsigned long long v) { return _lzcnt_u64(v); }

int stdc_leading_onesuc(unsigned char v) { return _lzcnt_u32(~v) - 24; }
int stdc_leading_onesus(unsigned short v) { return _lzcnt_u32(~v) - 16; }
int stdc_leading_onesui(unsigned int v) { return _lzcnt_u32(~v); }
int stdc_leading_onesul(unsigned long v) { return _lzcnt_u64(~v); }
int stdc_leading_onesull(unsigned long long v) { return _lzcnt_u64(~v); }

int stdc_trailing_zerosuc(unsigned char v) { return _tzcnt_u32(v); }
int stdc_trailing_zerosus(unsigned short v) { return _tzcnt_u32(v); }
int stdc_trailing_zerosui(unsigned int v) { return _tzcnt_u32(v); }
int stdc_trailing_zerosul(unsigned long v) { return _tzcnt_u64(v); }
int stdc_trailing_zerosull(unsigned long long v) { return _tzcnt_u64(v); }

int stdc_trailing_onesuc(unsigned char v) { return _tzcnt_u32(~v); }
int stdc_trailing_onesus(unsigned short v) { return _tzcnt_u32(~v); }
int stdc_trailing_onesui(unsigned int v) { return _tzcnt_u32(~v); }
int stdc_trailing_onesul(unsigned long v) { return _tzcnt_u64(~v); }
int stdc_trailing_onesull(unsigned long long v) { return _tzcnt_u64(~v); }


#define RETURN_BSR_RESULT(value)    \
    int dest = 0;                   \
    asm (                           \
        "bsr %0, %1\n"              \
        : "=r" (dest)               \
        : "r" (value)               \
    );                              \
    return dest

static inline int bsr16(unsigned short v) { RETURN_BSR_RESULT; }
static inline int bsr32(unsigned int v) { RETURN_BSR_RESULT; }
static inline int bsr64(unsigned long v) { RETURN_BSR_RESULT; }

int stdc_first_leading_zerouc(unsigned char v) { return bsr16(~v & 0xFF); }
int stdc_first_leading_zerous(unsigned short v) { return bsr16(~v); }
int stdc_first_leading_zeroui(unsigned int v) { return bsr32(~v); }
int stdc_first_leading_zeroul(unsigned long v) { return bsr64(~v); }
int stdc_first_leading_zeroull(unsigned long long v) { return bsr64(~v); }

int stdc_first_leading_oneuc(unsigned char v) { return bsr16(v); }
int stdc_first_leading_oneus(unsigned short v) { return bsr16(v); }
int stdc_first_leading_oneui(unsigned int v) { return bsr32(v); }
int stdc_first_leading_oneul(unsigned long v) { return bsr64(v); }
int stdc_first_leading_oneull(unsigned long long v) { return bsr64(v); }


#define RETURN_BSF_RESULT(value)    \
    int dest = 0;                   \
    asm (                           \
        "bsf %0, %1\n"              \
        : "=r" (dest)               \
        : "r" (value)               \
    );                              \
    return dest

static inline int bsf16(unsigned short v) { RETURN_BSF_RESULT; }
static inline int bsf32(unsigned int v) { RETURN_BSF_RESULT; }
static inline int bsf64(unsigned long v) { RETURN_BSF_RESULT; }

int stdc_first_trailing_zerouc(unsigned char v) { return bsf16(~v); }
int stdc_first_trailing_zerous(unsigned short v) { return bsf16(~v); }
int stdc_first_trailing_zeroui(unsigned int v) { return bsf32(~v); }
int stdc_first_trailing_zeroul(unsigned long v) { return bsf64(~v); }
int stdc_first_trailing_zeroull(unsigned long long v) { return bsf64(~v); }

int stdc_first_trailing_oneuc(unsigned char v) { return bsf16(v); }
int stdc_first_trailing_oneus(unsigned short v) { return bsf16(v); }
int stdc_first_trailing_oneui(unsigned int v) { return bsf32(v); }
int stdc_first_trailing_oneul(unsigned long v) { return bsf64(v); }
int stdc_first_trailing_oneull(unsigned long long v) { return bsf64(v); }


int stdc_count_zerosuc(unsigned char v) { return _popcnt32(v) - 24; }
int stdc_count_zerosus(unsigned short v) { return _popcnt32(v) - 16; }
int stdc_count_zerosui(unsigned int v) { return _popcnt32(v); }
int stdc_count_zerosul(unsigned long v) { return _popcnt64(v); }
int stdc_count_zerosull(unsigned long long v) { return _popcnt64(v); }

int stdc_count_onesuc(unsigned char v) { return _popcnt32(~v) - 24; }
int stdc_count_onesus(unsigned short v) { return _popcnt32(~v) - 16; }
int stdc_count_onesui(unsigned int v) { return _popcnt32(~v); }
int stdc_count_onesul(unsigned long v) { return _popcnt64(~v); }
int stdc_count_onesull(unsigned long long v) { return _popcnt64(~v); }

bool stdc_has_single_bituc(unsigned char v) { return v & 0x80; }
bool stdc_has_single_bitus(unsigned short v) { return v & 0x8000; }
bool stdc_has_single_bitui(unsigned int v) { return v & 0x80000000; }
bool stdc_has_single_bitul(unsigned long v) { return v & 0x8000000000000000; }
bool stdc_has_single_bitull(unsigned long long v) { return v & 0x8000000000000000; }

int stdc_bit_widthuc(unsigned char v) { return v ? 8 - stdc_leading_zerosuc(v) : 0; }
int stdc_bit_widthus(unsigned short v) { return v ? 16 - stdc_leading_zerosus(v) : 0; }
int stdc_bit_widthui(unsigned int v) { return v ? 32 - stdc_leading_zerosui(v) : 0; }
int stdc_bit_widthul(unsigned long v) { return v ? 64 - stdc_leading_zerosul(v) : 0; }
int stdc_bit_widthull(unsigned long long v) { return v ? 64 - stdc_leading_zerosull(v) : 0; }

unsigned char stdc_bit_flooruc(unsigned char v) { return v ? 1 << (stdc_bit_widthuc(v) - 1) : 0; }
unsigned short stdc_bit_floorus(unsigned short v) { return v ? 1 << (stdc_bit_widthus(v) - 1) : 0; }
unsigned int stdc_bit_floorui(unsigned int v) { return v ? 1 << (stdc_bit_widthui(v) - 1) : 0; }
unsigned long stdc_bit_floorul(unsigned long v) { return v ? 1 << (stdc_bit_widthul(v) - 1) : 0; }
unsigned long long stdc_bit_floorull(unsigned long long v) { return v ? 1 << (stdc_bit_widthull(v) - 1) : 0; }

unsigned char stdc_bit_ceiluc(unsigned char v) { return v ? 1 << stdc_bit_widthuc(v) : 0; }
unsigned short stdc_bit_ceilus(unsigned short v) { return v ? 1 << stdc_bit_widthus(v) : 0; }
unsigned int stdc_bit_ceilui(unsigned int v) { return v ? 1 << stdc_bit_widthui(v) : 0; }
unsigned long stdc_bit_ceilul(unsigned long v) { return v ? 1 << stdc_bit_widthul(v) : 0; }
unsigned long long stdc_bit_ceilull(unsigned long long v) { return v ? 1 << stdc_bit_widthull(v) : 0; }
