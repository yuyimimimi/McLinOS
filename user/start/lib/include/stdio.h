#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h> // For size_t
#include <stdarg.h> // For va_list
#include <stdint.h> // For int_max_t, etc. (though not strictly for basic stdio)

#ifdef __cplusplus
extern "C" {
#endif

// --- Macros and Constants ---
typedef int mode_t;
typedef long ssize_t; 


// Standard file descriptors
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

// End-of-file indicator
#ifndef EOF
#define EOF             (-1)
#endif

// Null pointer, already defined in stddef.h, but common to include here
#ifndef NULL
#define NULL            ((void*)0)
#endif

// Buffer sizes (can be adjusted)
#define BUFSIZ          1024

// File buffering modes
#define _IOFBF          0       // Fully buffered
#define _IOLBF          1       // Line buffered
#define _IONBF          2       // Unbuffered

// SEEK modes for fseek
#define SEEK_SET        0       // Seek from beginning of file
#define SEEK_CUR        1       // Seek from current position
#define SEEK_END        2       // Seek from end of file

// Maximum number of open files (can be adjusted based on your OS limits)
#define FOPEN_MAX       16

// L_tmpnam and TMP_MAX are usually for temporary file names, not essential for bare metal initially.

// --- FILE structure ---
// This is a simplified FILE structure. A full FILE structure
// would contain more fields for buffering, error flags, file position, etc.
typedef struct {
    int     fd;             // File descriptor from _open
    int     flags;          // Flags for mode (read/write/append, etc.) and buffering
    int     buf_mode;       // Buffering mode (_IOFBF, _IOLBF, _IONBF)
    char* buffer;         // Pointer to internal buffer
    size_t  buf_size;       // Size of the buffer
    size_t  buf_pos;        // Current position in the buffer (for reads/writes)
    size_t  buf_count;      // Number of valid bytes in buffer (for reads)
    int     ungetc_char;    // For ungetc() functionality
    int     eof_flag;       // End-of-file indicator
    int     err_flag;       // Error indicator
    // Potentially more fields for mutexes, wide characters, etc.
} FILE;



typedef union _G_fpos64_t {
	char __opaque[16];
	long long __lldata;
	double __align;
} fpos_t;



// --- Standard file streams ---
extern FILE __stdin;
extern FILE __stdout;
extern FILE __stderr;

#define stdin           (&__stdin)
#define stdout          (&__stdout)
#define stderr          (&__stderr)

// --- File operations ---

// Opens a file and associates a stream with it.
// mode: "r", "w", "a", "rb", "wb", "ab", "r+", "w+", "a+"
FILE* fopen(const char* filename, const char* mode);

// Closes the stream and flushes any buffered output.
int fclose(FILE* stream);

// Flushes the output buffer of the stream.
int fflush(FILE* stream);

// Sets the buffering mode for the stream.
void setbuf(FILE* stream, char* buffer);
int setvbuf(FILE* stream, char* buffer, int mode, size_t size);

// --- Character I/O ---

// Reads a character from the stream.
int fgetc(FILE* stream);
int getc(FILE* stream); // Macro version of fgetc
int getchar(void);      // Equivalent to getc(stdin)

// Writes a character to the stream.
int fputc(int c, FILE* stream);
int putc(int c, FILE* stream);  // Macro version of fputc
int putchar(int c);             // Equivalent to putc(c, stdout)

// Pushes a character back onto the input stream.
int ungetc(int c, FILE* stream);

// --- Line I/O ---

// Reads a line from the stream into `s`.
char* fgets(char* s, int size, FILE* stream);

// Writes a string to the stream.
int fputs(const char* s, FILE* stream);
int puts(const char* s); // Writes string to stdout followed by a newline

// --- Formatted I/O (Output) ---

// Writes formatted output to the stream.
int fprintf(FILE* stream, const char* format, ...);
int printf(const char* format, ...); // Equivalent to fprintf(stdout, format, ...)
int sprintf(char* s, const char* format, ...); // Writes formatted output to a string

// Writes formatted output to a string using a va_list.
int vsprintf(char* s, const char* format, va_list arg);
int vfprintf(FILE* stream, const char* format, va_list arg);
int vprintf(const char* format, va_list arg); // Equivalent to vfprintf(stdout, format, ...)

// --- Formatted I/O (Input) ---
// These are significantly more complex and often implemented later.
// int fscanf(FILE *stream, const char *format, ...);
// int scanf(const char *format, ...);
// int sscanf(const char *s, const char *format, ...);
// int vfscanf(FILE *stream, const char *format, va_list arg);
// int vscanf(const char *format, va_list arg);
// int vsscanf(const char *s, const char *format, va_list arg);

// --- Direct I/O (Block I/O) ---
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);

// --- File positioning functions ---
long ftell(FILE* stream);
int fseek(FILE* stream, long offset, int whence);
void rewind(FILE* stream);
void fsetpos(FILE* stream, const fpos_t* pos); // fpos_t needs to be defined
void fgetpos(FILE* stream, fpos_t* pos);

// --- Error handling ---
void clearerr(FILE* stream);
int feof(FILE* stream);
int ferror(FILE* stream);
// int perror(const char *s); // Requires errno and error string support

// --- Temporary files (often omitted for basic bare-metal) ---
// char *tmpnam(char *s);
// FILE *tmpfile(void);
// int remove(const char *filename);
// int rename(const char *oldname, const char *newname);

// --- Internal helper function (not part of standard API, but useful) ---
extern int _printk_internal(char* buf, const char* fmt, va_list args); // From vsprintf.c


#ifdef __cplusplus
}
#endif

#endif // _STDIO_H


