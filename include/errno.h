#ifndef __GINKGO_ERRNO_H
#define __GINKGO_ERRNO_H

// the macros are already defined in errno.h
#include <linux/errno.h>

// interact with glibc in compile time
#define errno (__Ginkgo_errno())

#endif // __GINKGO_ERRNO_H
