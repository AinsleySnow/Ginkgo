#include "../include/stdarg.h"
#include <string.h>

// Copyright 2020 Rui Ueyama. Released under the MIT license.

void* __Ginkgo_va_arg_mem(__Ginkgo_va_elem* ap, int sz, int align)
{
    void* p = ap->overflow_arg_area;
    if (align > 8)
        p = (void*)(((unsigned long)p + 15) / 16 * 16);
    ap->overflow_arg_area = (void*)(((unsigned long)p + sz + 7) / 8 * 8);
    return p;
}

void* __Ginkgo_va_arg_gp(__Ginkgo_va_elem* ap, int sz, int align)
{
    if (ap->gp_offset >= 48)
        return __Ginkgo_va_arg_mem(ap, sz, align);

    void* r = ap->reg_save_area + ap->gp_offset;
    ap->gp_offset += 8;
    return r;
}

void* __Ginkgo_va_arg_fp(__Ginkgo_va_elem* ap, int sz, int align)
{
    if (ap->fp_offset >= 112)
        return __Ginkgo_va_arg_mem(ap, sz, align);

    void* r = ap->reg_save_area + ap->fp_offset;
    ap->fp_offset += 8;
    return r;
}

void __Ginkgo_va_end(__Ginkgo_va_elem* ap)
{
    memset(ap, 0, sizeof(__Ginkgo_va_elem));
}
