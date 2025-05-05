
#ifndef _ERROR_H
#define _ERROR_H

#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/mutex.h>

// Default buffer size for error messages
#define DEFAULT_ERRR_BUFFER_SIZE (512)

// Enum for defining the error exception modes
// Defines how the error will be handled when encountered
enum error_exception_mode {
    ERROR_SAVE_ONLY = 0,               // Only save the error information, no action taken.
    ERROR_SAVE_AND_PRINT_TO_LOG = 1,   // Save the error information and print it to the kernel log.
    ERROR_SAVE_AND_BLOCK = 2,          // Save the error and block execution until the error is handled.
    ERROR_SAVE_AND_DO_HOOK = 3,
};

typedef intptr_t (*error_hook_t)(void*,intptr_t);//如果target为null会被传入被触发的error_t*

typedef struct {
    char *name;                      // The name of the error source, typically the function or module name
    intptr_t errnum;                      // Error number or error code
    timer_t time;                    // Timer to record the time when the error occurred
    int line;                        // Line number where the error occurred in the source code
    size_t errmsg_size;              // Size of the error message string
    char *errmsg;                    // The error message string
    int error_count;                 // A counter for the number of times the error occurred
    enum error_exception_mode mode;  // The mode in which to handle the error
    void *target;                    // Pointer to the target object (could be used for debugging)
    size_t haper_size;               // Size of memory allocated for the error buffer (optional)
    struct mutex s_mutex;
    error_hook_t e_holk;     
} error_t;

// Function declarations for error handling system
extern error_t *l_error_create(void *target, char *name, size_t error_buffer_size, enum error_exception_mode mode);
extern void l_error_exception(error_t *error, intptr_t errnum, int line, char *errmsg, ...);
extern int l_output_error_message(error_t *error, char *buffer, int buffer_size, void (out_function)(char *, ...));
extern void l_delete_error(error_t *error);

// Simple default error handler for quick use,no recomandation for use in product code
static error_t *simple_default_error = NULL;

// Inline function declarations for simplified error handling macros
static __always_inline error_t *fast_create_Exception(void *target, char* name);
static __always_inline void delete_Exception(error_t *Exception);
static __always_inline intptr_t catchException_line(error_t *Exception, intptr_t err, int line);

// Macro for custom error exception handling, supporting variable arguments for message formatting
#define Custom_catchException(error, errnum, line, errmsg, ...) \
        l_error_exception(error, errnum, line, errmsg, ##__VA_ARGS__)

// Macros to initialize and finalize error handling service
#define error_service_init() \
        simple_default_error = fast_create_Exception(NULL, __FILE__)

#define error_service_exit() \
        delete_Exception(simple_default_error)

// Macros for exception handling
// Captures and handles errors with automatic line number and file location
#define Exception(__errnum) \
        catchException_line(simple_default_error, __errnum, __LINE__)

#define Exception_with_msg(__errnum, __errmsg, ...) \
        Custom_catchException(simple_default_error, __errnum, __LINE__, __errmsg, ##__VA_ARGS__)

// Macros to print the error message or retrieve the error message
#define Print_error() \
        print_Exception_message(simple_default_error)

#define get_error_message(buffer, buffer_size) \
        get_Exception_message(simple_default_error, buffer, buffer_size)

// Inline function for fast creation of an error instance
static __always_inline error_t *fast_create_Exception(void *target, char* name) {
    error_t *err = l_error_create(target, name, DEFAULT_ERRR_BUFFER_SIZE, ERROR_SAVE_AND_PRINT_TO_LOG);
    if (IS_ERR(err)) {
        return ERR_CAST(err); 
    }
    return err;
}

// Inline function to create a custom error instance with user-defined settings
static __always_inline error_t *create_Exception(void *target, char *name) {
    return l_error_create(target, name, DEFAULT_ERRR_BUFFER_SIZE, ERROR_SAVE_AND_PRINT_TO_LOG);
}

// Inline function to create a custom error instance with specific buffer size and handling mode
static __always_inline error_t *create_Custom_Exception(void *target, char *name, size_t error_buffer_size, enum error_exception_mode mode) {
    return l_error_create(target, name, error_buffer_size, mode);
}

// Inline function to delete an error instance
static __always_inline void delete_Exception(error_t *Exception) {
    l_delete_error(Exception);
}

// Inline function to catch and handle an exception with line number
static __always_inline intptr_t catchException_line(error_t *Exception, intptr_t err, int line) {
    l_error_exception(Exception, err, line, NULL);
    return err;
}

// Macro for fast exception catching with the current line number
#define fast_catchException(Exception, err) \
        catchException_line(Exception, err, __LINE__)

// Inline function to print the error message using printk
static __always_inline void print_Exception_message(error_t *Exception) {
    l_output_error_message(Exception, NULL, 0, printk);
}

// Inline function to retrieve the error message into a provided buffer
static __always_inline void get_Exception_message(error_t *Exception, char *buffer, size_t size) {
    l_output_error_message(Exception, buffer, size, NULL);
}

// Inline function to set the default exception handler, ensuring proper type compatibility
static __always_inline int set_default_Exception(error_t *Exception) {
    BUILD_BUG_ON(!__builtin_types_compatible_p(typeof(Exception), typeof(simple_default_error)));
    if (Exception == NULL)
        simple_default_error = Exception;
    if (simple_default_error != NULL) {
        Exception_with_msg(-1, "set_default_Exception: you can't input not available argument");
    }
    return -1;
}
static __always_inline void set_Exception_holk(error_t *Exception, error_hook_t holk) {
    if (holk == NULL) {
        Exception_with_msg(-1, "please use useful holk");
    }
    else{
    Exception->e_holk = holk;        
    }
}
static __always_inline void set_Exception_mode(error_t *Exception,enum error_exception_mode mode){
    if(Exception != NULL){
        Exception->mode = mode;
    }
}
static __always_inline void set_Exception_target(error_t *Exception,void *target){
    if(Exception != NULL){
        Exception->target = target;
    }
}

#define quick_set_Exception_holk(x) \
    set_Exception_holk(simple_default_error,x)

#endif 
