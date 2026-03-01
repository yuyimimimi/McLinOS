#ifndef _STRING_H
#define _STRING_H

#include <stddef.h> // For size_t

#ifdef __cplusplus
extern "C" {
#endif

// --- Memory manipulation functions ---

// Fills the first `n` bytes of the memory area pointed to by `s` with the constant byte `c`.
// Returns a pointer to the memory area `s`.
void* memset(void* s, int c, size_t n);

// Copies `n` bytes from memory area `src` to memory area `dest`.
// The memory areas must not overlap.
// Returns a pointer to `dest`.
void* memcpy(void* dest, const void* src, size_t n);

// Copies `n` bytes from memory area `src` to memory area `dest`.
// The memory areas may overlap.
// Returns a pointer to `dest`.
void* memmove(void* dest, const void* src, size_t n);

// Compares the first `n` bytes of memory area `s1` and memory area `s2`.
// Returns an integer less than, equal to, or greater than zero if the first `n` bytes of `s1`
// is found, respectively, to be less than, to match, or be greater than the first `n` bytes of `s2`.
int memcmp(const void* s1, const void* s2, size_t n);

// Locates the first occurrence of `c` (converted to an unsigned char) in the first `n` bytes
// of the memory area pointed to by `s`.
// Returns a pointer to the byte located, or NULL if no such byte exists within `n` bytes.
void* memchr(const void* s, int c, size_t n);

// --- String manipulation functions ---

// Copies the string pointed to by `src`, including the terminating null byte,
// to the buffer pointed to by `dest`.
// The strings may not overlap, and the destination string `dest` must be large enough
// to receive the copy.
// Returns a pointer to the destination string `dest`.
char* strcpy(char* dest, const char* src);

// Copies at most `n` bytes from the string pointed to by `src`, including the
// terminating null byte, to the buffer pointed to by `dest`.
// If the length of `src` is less than `n`, the remainder of `dest` is padded with null bytes.
// If the length of `src` is greater than or equal to `n`, the string `dest` will not
// be null-terminated.
// Returns a pointer to the destination string `dest`.
char* strncpy(char* dest, const char* src, size_t n);

// Appends the `src` string to the `dest` string, overwriting the null byte at the
// end of `dest`, and then adds a terminating null byte.
// The strings may not overlap.
// Returns a pointer to the destination string `dest`.
char* strcat(char* dest, const char* src);

// Appends at most `n` bytes from the `src` string to the `dest` string, overwriting
// the null byte at the end of `dest`, and then adds a terminating null byte.
// The `dest` string must have enough space for the result.
// Returns a pointer to the destination string `dest`.
char* strncat(char* dest, const char* src, size_t n);

// Compares the string `s1` and the string `s2`.
// Returns an integer less than, equal to, or greater than zero if `s1` is found,
// respectively, to be less than, to match, or be greater than `s2`.
int strcmp(const char* s1, const char* s2);

// Compares at most `n` bytes of string `s1` and string `s2`.
// Returns an integer less than, equal to, or greater than zero if the first `n` bytes
// of `s1` is found, respectively, to be less than, to match, or be greater than the
// first `n` bytes of `s2`.
int strncmp(const char* s1, const char* s2, size_t n);

// Calculates the length of the string `s`, excluding the terminating null byte.
// Returns the number of bytes in the string `s`.
size_t strlen(const char* s);

// Locates the first occurrence of the character `c` (converted to a char) in the string `s`.
// Returns a pointer to the located character, or NULL if the character is not found.
char* strchr(const char* s, int c);

// Locates the last occurrence of the character `c` (converted to a char) in the string `s`.
// Returns a pointer to the located character, or NULL if the character is not found.
char* strrchr(const char* s, int c);

// Locates the first occurrence in the string `s1` of any of the characters in the string `s2`.
// Returns a pointer to the character found, or NULL if no such character is found.
char* strpbrk(const char* s1, const char* s2);

// Calculates the length of the initial segment of `s` which consists entirely of characters from `accept`.
// Returns the number of characters in the initial segment of `s` which consist only of bytes from `accept`.
size_t strspn(const char* s, const char* accept);

// Calculates the length of the initial segment of `s` which consists entirely of characters NOT from `reject`.
// Returns the number of characters in the initial segment of `s` which consist only of bytes NOT from `reject`.
size_t strcspn(const char* s, const char* reject);

// Locates the first occurrence of the string `needle` in the string `haystack`.
// Returns a pointer to the beginning of the located substring, or NULL if the substring is not found.
char* strstr(const char* haystack, const char* needle);

// Computes the length of the string `s`. If the string contains a null byte within the first `maxlen` bytes,
// the function returns the number of bytes up to (but not including) the first null byte.
// If the string does not contain a null byte within the first `maxlen` bytes, the function returns `maxlen`.
// This function is useful for bounded string operations where the null termination is not guaranteed.
size_t strnlen(const char* s, size_t maxlen);


// --- Error message functions (optional, usually requires errno and error string tables) ---
// For a basic bare-metal system, these might be minimal or omitted initially.
// char* strerror(int errnum);

#ifdef __cplusplus
}
#endif

#endif // _STRING_H

