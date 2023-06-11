#ifndef __GINKGO_STDARG_H
#define __GINKGO_STDARG_H

#define __STDC_VERSION_STDARG_H__ 202311L

typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} __Ginkgo_va_elem;

typedef __Ginkgo_va_elem va_list[1];


#define va_arg(ap, ty) __Ginkgo_va_arg(ap, ty)
#define va_copy(from, to) ((from)[0] = (to)[0])
#define va_end(ap)
#define va_start(ap, param) __Ginkgo_va_start(ap, param)

#endif // __GINKGO_STDARG_H
