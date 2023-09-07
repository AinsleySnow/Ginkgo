#ifndef __GINKGO_STDARG_H
#define __GINKGO_STDARG_H

#define __STDC_VERSION_STDARG_H__ 202311L

typedef struct
{
    unsigned int gp_offset;
    unsigned int fp_offset;
    void* overflow_arg_area;
    void* reg_save_area;
} __Ginkgo_va_elem;

typedef __Ginkgo_va_elem va_list[1];

void* __Ginkgo_va_arg_mem(__Ginkgo_va_elem* ap, int sz, int align);
void* __Ginkgo_va_arg_gp(__Ginkgo_va_elem* ap, int sz, int align);
void* __Ginkgo_va_arg_fp(__Ginkgo_va_elem* ap, int sz, int align);

// In C23, va_start is a variadic macro, and only the first
// argument passed to va_start is evaluated. (7.16.1.4[4])
// I used a trick to avoid copying the __Ginkgo_va_elem struct.
#define va_start(ap, ...)   \
    __Ginkgo_va_start(&ap __VA_OPT__(,) __VA_ARGS__)

// __Ginkgo_va_arg is marked as built-in function and will
// be replaced by a variant above
#define va_arg(ap, ty)      (*(ty*)__Ginkgo_va_arg(&ap, ty))
#define va_copy(from, to)   ((from)[0] = (to)[0])
#define va_end(ap)          __Ginkgo_va_end(&ap)

#endif // __GINKGO_STDARG_H
