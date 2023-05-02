#ifndef __GINKGO_STDBIT_H
#define __GINKGO_STDBIT_H

#define __STDC_VERSION_STDBIT_H__ 202311L

int stdc_leading_zerosuc(unsigned char v);
int stdc_leading_zerosus(unsigned short v);
int stdc_leading_zerosui(unsigned int v);
int stdc_leading_zerosul(unsigned long v);
int stdc_leading_zerosull(unsigned long long v);
#define stdc_leading_zero(v)                        \
_Gerenic((v),                                       \
    unsigned char: stdc_leading_zerosuc(v),         \
    unsigned short: stdc_leading_zerosus(v),        \
    unsigned int: stdc_leading_zerosui(v),          \
    unsigned long: stdc_leading_zerosul(v),         \
    unsigned long long: stdc_leading_zerosull(v)    \
)

int stdc_leading_onesuc(unsigned char v);
int stdc_leading_onesus(unsigned short v);
int stdc_leading_onesui(unsigned int v);
int stdc_leading_onesul(unsigned long v);
int stdc_leading_onesull(unsigned long long v);
#define stdc_leading_one(v)                         \
_Gerenic((v),                                       \
    unsigned char: stdc_leading_onesuc(v),          \
    unsigned short: stdc_leading_onesus(v),         \
    unsigned int: stdc_leading_onesui(v),           \
    unsigned long: stdc_leading_onesul(v),          \
    unsigned long long: stdc_leading_onesull(v)     \
)

int stdc_trailing_zerosuc(unsigned char v);
int stdc_trailing_zerosus(unsigned short v);
int stdc_trailing_zerosui(unsigned int v);
int stdc_trailing_zerosul(unsigned long v);
int stdc_trailing_zerosull(unsigned long long v);
#define stdc_trailing_zero(v)                       \
_Gerenic((v),                                       \
    unsigned char: stdc_trailing_zerosuc(v),        \
    unsigned short: stdc_trailing_zerosus(v),       \
    unsigned int: stdc_trailing_zerosui(v),         \
    unsigned long: stdc_trailing_zerosul(v),        \
    unsigned long long: stdc_trailing_zerosull(v)   \
)

int stdc_trailing_onesuc(unsigned char v);
int stdc_trailing_onesus(unsigned short v);
int stdc_trailing_onesui(unsigned int v);
int stdc_trailing_onesul(unsigned long v);
int stdc_trailing_onesull(unsigned long long v);
#define stdc_trailing_one(v)                       \
_Gerenic((v),                                      \
    unsigned char: stdc_trailing_onesuc(v),        \
    unsigned short: stdc_trailing_onesus(v),       \
    unsigned int: stdc_trailing_onesui(v),         \
    unsigned long: stdc_trailing_onesul(v),        \
    unsigned long long: stdc_trailing_onesull(v)   \
)

int stdc_first_leading_zerouc(unsigned char v);
int stdc_first_leading_zerous(unsigned short v);
int stdc_first_leading_zeroui(unsigned int v);
int stdc_first_leading_zeroul(unsigned long v);
int stdc_first_leading_zeroull(unsigned long long v);
#define stdc_first_leading_zero(v)                      \
_Gerenic((v),                                           \
    unsigned char: stdc_first_leading_zerouc(v),        \
    unsigned short: stdc_first_leading_zerous(v),       \
    unsigned int: stdc_first_leading_zeroui(v),         \
    unsigned long: stdc_first_leading_zeroul(v),        \
    unsigned long long: stdc_first_leading_zeroull(v)   \
)

int stdc_first_leading_oneuc(unsigned char v);
int stdc_first_leading_oneus(unsigned short v);
int stdc_first_leading_oneui(unsigned int v);
int stdc_first_leading_oneul(unsigned long v);
int stdc_first_leading_oneull(unsigned long long v);
#define stdc_first_leading_one(v)                       \
_Gerenic((v),                                           \
    unsigned char: stdc_first_leading_oneuc(v),         \
    unsigned short: stdc_first_leading_oneus(v),        \
    unsigned int: stdc_first_leading_oneui(v),          \
    unsigned long: stdc_first_leading_oneul(v),         \
    unsigned long long: stdc_first_leading_oneull(v)    \
)

int stdc_first_trailing_zerouc(unsigned char v);
int stdc_first_trailing_zerous(unsigned short v);
int stdc_first_trailing_zeroui(unsigned int v);
int stdc_first_trailing_zeroul(unsigned long v);
int stdc_first_trailing_zeroull(unsigned long long v);
#define stdc_first_trailing_zero(v)                     \
_Gerenic((v),                                           \
    unsigned char: stdc_first_trailing_zerouc(v),       \
    unsigned short: stdc_first_trailing_zerous(v),      \
    unsigned int: stdc_first_trailing_zeroui(v),        \
    unsigned long: stdc_first_trailing_zeroul(v),       \
    unsigned long long: stdc_first_trailing_zeroull(v)  \
)

int stdc_first_trailing_oneuc(unsigned char v);
int stdc_first_trailing_oneus(unsigned short v);
int stdc_first_trailing_oneui(unsigned int v);
int stdc_first_trailing_oneul(unsigned long v);
int stdc_first_trailing_oneull(unsigned long long v);
#define stdc_first_trailing_one(v)                      \
_Gerenic((v),                                           \
    unsigned char: stdc_first_trailing_oneuc(v),        \
    unsigned short: stdc_first_trailing_oneus(v),       \
    unsigned int: stdc_first_trailing_oneui(v),         \
    unsigned long: stdc_first_trailing_oneul(v),        \
    unsigned long long: stdc_first_trailing_oneull(v)   \
)

int stdc_count_zerosuc(unsigned char v);
int stdc_count_zerosus(unsigned short v);
int stdc_count_zerosui(unsigned int v);
int stdc_count_zerosul(unsigned long v);
int stdc_count_zerosull(unsigned long long v);
#define stdc_count_zero(v)                          \
_Gerenic((v),                                       \
    unsigned char: stdc_count_zerosuc(v),           \
    unsigned short: stdc_count_zerosus(v),          \
    unsigned int: stdc_count_zerosui(v),            \
    unsigned long: stdc_count_zerosul(v),           \
    unsigned long long: stdc_count_zerosull(v)      \
)

int stdc_count_onesuc(unsigned char v);
int stdc_count_onesus(unsigned short v);
int stdc_count_onesui(unsigned int v);
int stdc_count_onesul(unsigned long v);
int stdc_count_onesull(unsigned long long v);
#define stdc_count_one(v)                           \
_Gerenic((v),                                       \
    unsigned char: stdc_count_onesuc(v),            \
    unsigned short: stdc_count_onesus(v),           \
    unsigned int: stdc_count_onesui(v),             \
    unsigned long: stdc_count_onesul(v),            \
    unsigned long long: stdc_count_onesull(v)       \
)

bool stdc_has_single_bituc(unsigned char v);
bool stdc_has_single_bitus(unsigned short v);
bool stdc_has_single_bitui(unsigned int v);
bool stdc_has_single_bitul(unsigned long v);
bool stdc_has_single_bitull(unsigned long long v);
#define stdc_has_single_bit(v)                      \
_Gerenic((v),                                       \
    unsigned char: stdc_has_single_bituc(v),        \
    unsigned short: stdc_has_single_bitus(v)        \
    unsigned int: stdc_has_single_bitui(v),         \
    unsigned long: stdc_has_single_bitul(v),        \
    unsigned long long: stdc_has_single_bitull(v)   \
)

int stdc_bit_widthuc(unsigned char v);
int stdc_bit_widthus(unsigned short v);
int stdc_bit_widthui(unsigned int v);
int stdc_bit_widthul(unsigned long v);
int stdc_bit_widthull(unsigned long long v);
#define stdc_bit_width(v)                           \
_Gerenic((v),                                       \
    unsigned char: stdc_bit_widthuc(v),             \
    unsigned short: stdc_bit_widthus(v),            \
    unsigned int: stdc_bit_widthui(v),              \
    unsigned long: stdc_bit_widthul(v),             \
    unsigned long long: stdc_bit_widthull(v)        \
)

unsigned char stdc_bit_flooruc(unsigned char v);
unsigned short stdc_bit_floorus(unsigned short v);
unsigned int stdc_bit_floorui(unsigned int v);
unsigned long stdc_bit_floorul(unsigned long v);
unsigned long long stdc_bit_floorull(unsigned long long v);
#define stdc_bit_floor(v)                           \
_Gerenic((v),                                       \
    unsigned char: stdc_bit_flooruc(v),             \
    unsigned short: stdc_bit_floorus(v),            \
    unsigned int: stdc_bit_floorui(v),              \
    unsigned long: stdc_bit_floorul(v),             \
    unsigned long long: stdc_bit_floorull(v)        \
)

unsigned char stdc_bit_ceiluc(unsigned char v);
unsigned short stdc_bit_ceilus(unsigned short v);
unsigned int stdc_bit_ceilui(unsigned int v);
unsigned long stdc_bit_ceilul(unsigned long v);
unsigned long long stdc_bit_ceilull(unsigned long long v);
#define stdc_bit_ceil(v)                            \
_Gerenic((v),                                       \
    unsigned char: stdc_bit_ceiluc(v),              \
    unsigned short: stdc_bit_ceilus(v),             \
    unsigned int: stdc_bit_ceilui(v),               \
    unsigned long: stdc_bit_ceilul(v),              \
    unsigned long long: stdc_bit_ceilull(v)         \
)

#endif // __GINKGO_STDBIT_H
