#ifndef __GINKGO_SIGNAL_H
#define __GINKGO_SIGNAL_H

typedef int sig_atomic_t;

#define SIG_DFL 0
#define SIG_ERR -1
#define SIG_IGN 1

#define SIGABRT 6
#define SIGFPE 8
#define SIGILL 4
#define SIGINT 2
#define SIGSEGV 11
#define SIGTERM 15

void (*signal(int sig, void (*func)(int)))(int);
int raise(int sig);

#endif // __GINKGO_SIGNAL_H
