#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h> // For size_t

#ifdef __cplusplus
extern "C" {
#endif

// --- Memory Management Functions ---

void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);

// --- String Conversion Functions ---

int atoi(const char* str);
long atol(const char* str);
long long atoll(const char* str);

double atof(const char* str); // Often requires float.h and math.h dependencies

long strtol(const char* str, char** endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);
long long strtoll(const char* str, char** endptr, int base);
unsigned long long strtoull(const char* str, char** endptr, int base);

// --- Pseudo-random Number Generation Functions ---

#define RAND_MAX 32767 // Typical value, can be adjusted

int rand(void);
void srand(unsigned int seed);

// --- Communication with the Environment Functions ---

// For bare-metal, these usually trigger a system halt or reboot.
void abort(void) __attribute__((noreturn));
void exit(int status) __attribute__((noreturn));
// int atexit(void (*func)(void)); // Requires exit handler management
// char* getenv(const char* name); // Requires environment variable support
// int system(const char* command); // Requires command interpreter/shell

// --- Searching and Sorting Functions ---

void* bsearch(const void* key, const void* base, size_t nmemb, size_t size,
              int (*compar)(const void*, const void*));
void qsort(void* base, size_t nmemb, size_t size,
           int (*compar)(const void*, const void*));

// --- Integer Arithmetic Functions ---

typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;
typedef struct { long long quot; long long rem; } lldiv_t;

div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);
lldiv_t lldiv(long long numer, long long denom);

// --- Utility Macros ---

#ifndef __cplusplus
#define abs(x) __builtin_abs(x)
#define labs(x) __builtin_labs(x)
#define llabs(x) __builtin_llabs(x)
#else
// For C++ linkage, these might be overloaded functions
extern int abs(int x);
extern long labs(long x);
extern long long llabs(long long x);
#endif

#ifdef __cplusplus
}
#endif

#endif // _STDLIB_H