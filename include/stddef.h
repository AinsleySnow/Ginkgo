#ifndef __GINKGO_STDDEF_H
#define __GINKGO_STDDEF_H

#define __STDC_VERSION_STDDEF_H__ 202311L

typedef long long ptrdiff_t;
typedef unsigned long long size_t;
typedef long long max_align_t;

typedef int wchar_t;

typedef typeof_unqual(nullptr) nullptr_t;
#define NULL nullptr

#define unreachable() __Ginkgo_unreachable()
#define offsetof(type, member) __Ginkgo_offsetof(type, member)

#endif // __GINKGO_STDDEF_H
