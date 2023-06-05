#ifndef __GINKGO_STRING_H
#define __GINKGO_STRING_H

#include <stddef.h>

void *memcpy(void * restrict s1, const void * restrict s2, size_t n);
void *memccpy(void * restrict s1, const void * restrict s2, int c, size_t n);
void *memmove(void *s1, const void *s2, size_t n);

char *strcpy(char * restrict s1, const char * restrict s2);
char *strncpy(char * restrict s1, const char * restrict s2, size_t n);

char *strdup(const char *s);
char *strndup(const char *s, size_t size);

char *strcat(char * restrict s1, const char * restrict s2);
char *strncat(char * restrict s1, const char * restrict s2, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);
int strcmp(const char *s1, const char *s2);
int strcoll(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strxfrm(char * restrict s1, const char * restrict s2, size_t n);

void *memchr(void *s, int c, size_t n);
char *strchr(char *s, int c);

size_t strcspn(const char *s1, const char *s2);
char *strpbrk(char *s1, const char *s2);
char *strrchr(char *s, int c);
size_t strspn(const char *s1, const char *s2);
char *strstr(char *s1, const char *s2);

char *strtok(char * restrict s1, const char * restrict s2);

void *memset(void *s, int c, size_t n);
void *memset_explicit(void *s, int c, size_t n);

char *strerror(int errnum);
size_t strlen(const char *s);

#endif // __GINKGO_STRING_H
