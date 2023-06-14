#ifndef __GINKGO_FENV_H
#define __GINKGO_FENV_H

#define __STDC_VERSION_FENV_H__ 202311L

// Workarounds to get fenv.h correctly loaded.
#ifndef _FENV_H
#define _FENV_H
#endif // _FENV_H
#ifndef __x86_64__
#define __x86_64__
#endif // __x86_64__

#include <bits/fenv.h>

// Is this just a lite fenv_t? I don't understand.
typedef struct
{
    unsigned short __control_word;
    // make glibc happy
    unsigned: 16;
    int __mxcsr;
} femode_t;


int feclearexcept(int excepts);
int fegetexceptflag(fexcept_t *flagp, int excepts);
int feraiseexcept(int excepts);
int fesetexcept(int excepts);

int fesetexceptflag(const fexcept_t *flagp, int excepts);
int fetestexceptflag(const fexcept_t * flagp, int excepts);
int fetestexcept(int excepts);

int fegetmode(femode_t *modep);
int fesetmode(const femode_t *modep);

int fegetround(void);
int fesetround(int rnd);

int fegetenv(fenv_t *envp);
int fesetenv(const fenv_t *envp);
int feholdexcept(fenv_t *envp);
int feupdateenv(const fenv_t *envp);

#endif // __GINKGO_FENV_H
