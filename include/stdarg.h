#ifndef __GINKGO_STDARG_H
#define __GINKGO_STDARG_H

#define __STDC_VERSION_STDARG_H__ 202311L

typedef struct
{
    unsigned int gp_offset;
    unsigned int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} __Ginkgo_va_elem;

typedef __Ginkgo_va_elem va_list[1];


static void *__va_arg_mem(__va_elem *ap, int sz, int align)
{
    void *p = ap->overflow_arg_area;
    if (align > 8)
        p = (p + 15) / 16 * 16;
    ap->overflow_arg_area = ((unsigned long)p + sz + 7) / 8 * 8;
    return p;
}

static void *__va_arg_gp(__va_elem *ap, int sz, int align)
{
    if (ap->gp_offset >= 48)
        return __va_arg_mem(ap, sz, align);

    void *r = ap->reg_save_area + ap->gp_offset;
    ap->gp_offset += 8;
    return r;
}

static void *__va_arg_fp(__va_elem *ap, int sz, int align)
{
    if (ap->fp_offset >= 112)
        return __va_arg_mem(ap, sz, align);

    void *r = ap->reg_save_area + ap->fp_offset;
    ap->fp_offset += 8;
    return r;
}


#define va_arg(ap, ty)      (*(ty*)__Ginkgo_va_arg(ap, ty))
#define va_copy(from, to)   ((from)[0] = (to)[0])
#define va_end(ap)          __Ginkgo_va_end(ap)
#define va_start(ap, param) __Ginkgo_va_start(ap, param)

#endif // __GINKGO_STDARG_H
