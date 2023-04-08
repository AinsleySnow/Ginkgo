#ifndef __GINKGO_ASSERT_H
#define __GINKGO_ASSERT_H

#define __STDC_VERSION_ASSERT_H__ 202311L

# ifdef NDEBUG
#   define assert(expr) ((void)0)
# else
#   define assert(expr) __Ginkgo_assert(#expr, __FILE__, __LINE__, __func__)
# endif // NDEBUG

#endif // __GINKGO_ASSERT_H
