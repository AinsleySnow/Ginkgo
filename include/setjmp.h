#ifndef __GINKGO_SETJMP_H
#define __GINKGO_SETJMP_H

#define __STDC_VERSION_SETJMP_H__ 202311L

#include <bits/types/struct___jmp_buf_tag.h>

typedef struct __jmp_buf_tag jmp_buf[1];

int setjmp(jmp_buf env);
[[noreturn]] void longjmp(jmp_buf env, int val);

#endif // __GINKGO_SETJMP_H
