#ifndef __GINKGO_STDCKDINT_H
#define __GINKGO_STDCKDINT_H

#define __STDC_VERSION_STDCKDINT_H__ 202311L

#define ckd_add(result, lhs, rhs) __Ginkgo_ckd_add(result, (lhs), (rhs))
#define ckd_sub(result, lhs, rhs) __Ginkgo_ckd_sub(result, (lhs), (rhs))
#define ckd_mul(result, lhs, rhs) __Ginkgo_ckd_mul(result, (lhs), (rhs))

#endif // __GINKGO_STDCKDINT_H
