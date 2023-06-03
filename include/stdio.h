#ifndef __GINKGO_STDIO_H
#define __GINKGO_STDIO_H

#define __STDC_VERSION_STDIO_H__ 202311L

#include <stdarg.h>
#include <stddef.h>

// defination of FILE
#include <bits/types/FILE.h>
// defination of fpos_t
#include <bits/types/__fpos_t.h>
// ... since __fpos_t.h only contains defination of __fpos_t.
typedef __fpos_t fpos_t;

// consistant with those defined in GCC
#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

// The buffer size used by setbuf.
// Consistant with that defined in GCC stdio.h.
#define BUFSIZ 8192

// end of file indicator
#define EOF (-1)

#define FOPEN_MAX 16
#define FILENAME_MAX 4096
#define L_tmpnam 20

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0

#define TMP_MAX 238328

extern FILE* stderr;
extern FILE* stdin;
extern FILE* stdout;

#define _PRINTF_NAN_LEN_MAX 4

int remove(const char *filename);
int rename(const char *old, const char *new);
FILE *tmpfile(void);
char *tmpnam(char *s);

int fclose(FILE *stream);
int fflush(FILE *stream);
FILE *fopen(const char * restrict filename, const char * restrict mode);
FILE *freopen(const char * restrict filename, const char * restrict mode, FILE * restrict stream);

void setbuf(FILE * restrict stream, char * restrict buf);
int setvbuf(FILE * restrict stream, char * restrict buf, int mode, size_t size);

int printf(const char * restrict format, ...);
int scanf(const char * restrict format, ...);

int snprintf(char * restrict s, size_t n, const char * restrict format, ...);
int sprintf(char * restrict s, const char * restrict format, ...);
int sscanf(const char * restrict s, const char * restrict format, ...);

int vfprintf(FILE * restrict stream, const char * restrict format, va_list arg);
int vfscanf(FILE * restrict stream, const char * restrict format, va_list arg);

int vprintf(const char * restrict format, va_list arg);
int vscanf(const char * restrict format, va_list arg);

int vsnprintf(char * restrict s, size_t n, const char * restrict format, va_list arg);
int vsprintf(char * restrict s, const char * restrict format, va_list arg);
int vsscanf(const char * restrict s, const char * restrict format, va_list arg);

int fgetc(FILE *stream);
char *fgets(char * restrict s, int n, FILE * restrict stream);
int fputc(int c, FILE *stream);
int fputs(const char * restrict s, FILE * restrict stream);

int getc(FILE *stream);
int getchar(void);
int putc(int c, FILE *stream);
int putchar(int c);
int puts(const char *s);
int ungetc(int c, FILE *stream);

size_t fread(void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream);
size_t fwrite(const void * restrict ptr, size_t size, size_t nmemb, FILE * restrict stream);
int fgetpos(FILE * restrict stream, fpos_t * restrict pos);
int fseek(FILE *stream, long int offset, int whence);
int fsetpos(FILE *stream, const fpos_t *pos);
long int ftell(FILE *stream);
void rewind(FILE *stream);

void clearerr(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void perror(const char *s);
int fprintf(FILE * restrict stream, const char * restrict format, ...);
int fscanf(FILE * restrict stream, const char * restrict format, ...);

#endif // __GINKGO_STDIO_H
