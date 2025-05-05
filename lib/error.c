#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/error.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/stdarg.h>
#include <linux/sprintf.h>


error_t *l_error_create(void *target,char *name,size_t error_buffer_size,enum error_exception_mode mode)
{
    error_t *error = (error_t *)kmalloc(sizeof(error_t), GFP_KERNEL);
    if(error == NULL)return (error_t*)-MAX_ERRNO;
 
    if(error_buffer_size <= 0) error_buffer_size = DEFAULT_ERRR_BUFFER_SIZE;
    error->errmsg = (char *)kmalloc(error_buffer_size, GFP_KERNEL);
    if(error->errmsg == NULL){
        kfree(error);
        return (error_t*) -ENOMEM;
    }
    error->errmsg_size = error_buffer_size; 
 
    if(name == NULL) name = __FILE__;
    int name_length = strlen(name);
    error->name = (char *)kmalloc(name_length+1, GFP_KERNEL);
    if(error->name == NULL){
        kfree(error->errmsg);
        kfree(error);
        return (error_t*)-ENOMEM;
    }        
    strcpy(error->name, name);

    error->target = target;
    error->error_count = 0;
    error->mode = mode;
    return error;
}

void l_delete_error(error_t *error)
{
    if(error == NULL) return;
    if(error->name != NULL) kfree(error->name);
    if(error->errmsg != NULL) kfree(error->errmsg);
    kfree(error);
}

static void __write_error_message(error_t *error,char *errmsg,...){
    if(error == NULL) return;
    if(errmsg == NULL) return;
    va_list args;
    va_start(args, errmsg);
    vsnprintf(error->errmsg,error->errmsg_size,errmsg,args);
    va_end(args);
    return;
}

extern size_t get_global_heap_size(void);
void l_error_exception(error_t *error, intptr_t errnum, int line, char *errmsg, ...) 
{
    if(error == NULL) return;
    if(errnum >= 0) return;
    error->errnum = errnum;
    error->line = line;
    error->time = ktime_get();
    error->haper_size = get_global_heap_size();
    error->error_count++;
    if (errmsg != NULL) {
        va_list args;
        va_start(args, errmsg);
        vsnprintf(error->errmsg, error->errmsg_size, errmsg, args);
        va_end(args);
    } 
    else 
    {
        switch (errnum) {
            case -EPERM:
                __write_error_message(error, "ERROR: EPERM: Operation not permitted");
                break;
            case -ENOENT:
                __write_error_message(error, "ERROR: ENOENT: No such file or directory");
                break;
            case -ESRCH:
                __write_error_message(error, "ERROR: ESRCH: No such process");
                break;
            case -EINTR:
                __write_error_message(error, "WARN: EINTR: Interrupted system call");
                break;
            case -EIO:
                __write_error_message(error, "ERROR: EIO: I/O error");
                break;
            case -ENXIO:
                __write_error_message(error, "ERROR: ENXIO: No such device or address");
                break;
            case -E2BIG:
                __write_error_message(error, "ERROR: E2BIG: Argument list too long");
                break;
            case -ENOEXEC:
                __write_error_message(error, "ERROR: ENOEXEC: Exec format error");
                break;
            case -EBADF:
                __write_error_message(error, "ERROR: EBADF: Bad file descriptor");
                break;
            case -ECHILD:
                __write_error_message(error, "ERROR: ECHILD: No child processes");
                break;
            case -EAGAIN:
                __write_error_message(error, "WARN: EAGAIN: Resource temporarily unavailable");
                break;
            case -ENOMEM:
                __write_error_message(error, "ERROR: ENOMEM: Out of memory");
                break;
            case -EACCES:
                __write_error_message(error, "ERROR: EACCES: Permission denied");
                break;
            case -EFAULT:
                __write_error_message(error, "ERROR: EFAULT: Bad address");
                break;
            case -ENOTBLK:
                __write_error_message(error, "ERROR: ENOTBLK: Block device required");
                break;
            case -EBUSY:
                __write_error_message(error, "ERROR: EBUSY: Device or resource busy");
                break;
            case -EEXIST:
                __write_error_message(error, "ERROR: EEXIST: File exists");
                break;
            case -EXDEV:
                __write_error_message(error, "ERROR: EXDEV: Cross-device link");
                break;
            case -ENODEV:
                __write_error_message(error, "ERROR: ENODEV: No such device");
                break;
            case -ENOTDIR:
                __write_error_message(error, "ERROR: ENOTDIR: Not a directory");
                break;
            case -EISDIR:
                __write_error_message(error, "ERROR: EISDIR: Is a directory");
                break;
            case -EINVAL:
                __write_error_message(error, "ERROR: EINVAL: Invalid argument");
                break;
            case -ENFILE:
                __write_error_message(error, "ERROR: ENFILE: File table overflow");
                break;
            case -EMFILE:
                __write_error_message(error, "ERROR: EMFILE: Too many open files");
                break;
            case -ENOTTY:
                __write_error_message(error, "ERROR: ENOTTY: Not a terminal");
                break;
            case -ETXTBSY:
                __write_error_message(error, "ERROR: ETXTBSY: Text file busy");
                break;
            case -EFBIG:
                __write_error_message(error, "ERROR: EFBIG: File too large");
                break;
            case -ENOSPC:
                __write_error_message(error, "ERROR: ENOSPC: No space left on device");
                break;
            case -ESPIPE:
                __write_error_message(error, "ERROR: ESPIPE: Illegal seek");
                break;
            case -EROFS:
                __write_error_message(error, "ERROR: EROFS: Read-only file system");
                break;
            case -EMLINK:
                __write_error_message(error, "ERROR: EMLINK: Too many links");
                break;
            case -EPIPE:
                __write_error_message(error, "ERROR: EPIPE: Broken pipe");
                break;
            case -EDOM:
                __write_error_message(error, "ERROR: EDOM: Math argument out of domain");
                break;
            case -ERANGE:
                __write_error_message(error, "ERROR: ERANGE: Math result not representable");
                break;
        
            /* 扩展错误码 (from errno.h) */
            case -EDEADLK:
                __write_error_message(error, "ERROR: EDEADLK: Resource deadlock would occur");
                break;
            case -ENAMETOOLONG:
                __write_error_message(error, "ERROR: ENAMETOOLONG: File name too long");
                break;
            case -ENOLCK:
                __write_error_message(error, "ERROR: ENOLCK: No record locks available");
                break;
            case -ENOSYS:
                __write_error_message(error, "ERROR: ENOSYS: Function not implemented");
                break;
            case -ENOTEMPTY:
                __write_error_message(error, "ERROR: ENOTEMPTY: Directory not empty");
                break;
            case -ELOOP:
                __write_error_message(error, "ERROR: ELOOP: Too many symbolic links encountered");
                break;
            /* 网络相关错误码 */
            case -ENOTSOCK:
                __write_error_message(error, "ERROR: ENOTSOCK: Socket operation on non-socket");
                break;
            case -EDESTADDRREQ:
                __write_error_message(error, "ERROR: EDESTADDRREQ: Destination address required");
                break;
            case -EMSGSIZE:
                __write_error_message(error, "ERROR: EMSGSIZE: Message too long");
                break;
            case -EPROTOTYPE:
                __write_error_message(error, "ERROR: EPROTOTYPE: Protocol wrong type for socket");
                break;
            case -ENOPROTOOPT:
                __write_error_message(error, "ERROR: ENOPROTOOPT: Protocol not available");
                break;
            case -EPROTONOSUPPORT:
                __write_error_message(error, "ERROR: EPROTONOSUPPORT: Protocol not supported");
                break;
            case -ESOCKTNOSUPPORT:
                __write_error_message(error, "ERROR: ESOCKTNOSUPPORT: Socket type not supported");
                break;
            case -EOPNOTSUPP:
                __write_error_message(error, "ERROR: EOPNOTSUPP: Operation not supported");
                break;
            case -EPFNOSUPPORT:
                __write_error_message(error, "ERROR: EPFNOSUPPORT: Protocol family not supported");
                break;
            case -EAFNOSUPPORT:
                __write_error_message(error, "ERROR: EAFNOSUPPORT: Address family not supported");
                break;
            case -EADDRINUSE:
                __write_error_message(error, "ERROR: EADDRINUSE: Address already in use");
                break;
            case -EADDRNOTAVAIL:
                __write_error_message(error, "ERROR: EADDRNOTAVAIL: Cannot assign requested address");
                break;
            case -ENETDOWN:
                __write_error_message(error, "ERROR: ENETDOWN: Network is down");
                break;
            case -ENETUNREACH:
                __write_error_message(error, "ERROR: ENETUNREACH: Network is unreachable");
                break;
            case -ENETRESET:
                __write_error_message(error, "ERROR: ENETRESET: Network dropped connection on reset");
                break;
            case -ECONNABORTED:
                __write_error_message(error, "ERROR: ECONNABORTED: Software caused connection abort");
                break;
            case -ECONNRESET:
                __write_error_message(error, "ERROR: ECONNRESET: Connection reset by peer");
                break;
            case -ENOBUFS:
                __write_error_message(error, "ERROR: ENOBUFS: No buffer space available");
                break;
            case -EISCONN:
                __write_error_message(error, "ERROR: EISCONN: Transport endpoint is already connected");
                break;
            case -ENOTCONN:
                __write_error_message(error, "ERROR: ENOTCONN: Transport endpoint is not connected");
                break;
            case -ESHUTDOWN:
                __write_error_message(error, "ERROR: ESHUTDOWN: Cannot send after transport endpoint shutdown");
                break;
            case -ETOOMANYREFS:
                __write_error_message(error, "ERROR: ETOOMANYREFS: Too many references");
                break;
            case -ETIMEDOUT:
                __write_error_message(error, "ERROR: ETIMEDOUT: Connection timed out");
                break;
            case -ECONNREFUSED:
                __write_error_message(error, "ERROR: ECONNREFUSED: Connection refused");
                break;
            case -EHOSTDOWN:
                __write_error_message(error, "ERROR: EHOSTDOWN: Host is down");
                break;
            case -EHOSTUNREACH:
                __write_error_message(error, "ERROR: EHOSTUNREACH: No route to host");
                break;
            /* 其他错误码 */
            case -EALREADY:
                __write_error_message(error, "INFO: EALREADY: Operation already in progress");
                break;
            case -EINPROGRESS:
                __write_error_message(error, "INFO: EINPROGRESS: Operation now in progress");
                break;
            case -ESTALE:
                __write_error_message(error, "ERROR: ESTALE: Stale file handle");
                break;
            case -EUCLEAN:
                __write_error_message(error, "ERROR: EUCLEAN: Structure needs cleaning");
                break;
            case -ENOTNAM:
                __write_error_message(error, "ERROR: ENOTNAM: Not a XENIX named type file");
                break;
            case -ENAVAIL:
                __write_error_message(error, "ERROR: ENAVAIL: No XENIX semaphores available");
                break;
            case -EISNAM:
                __write_error_message(error, "ERROR: EISNAM: Is a named type file");
                break;
            case -EREMOTEIO:
                __write_error_message(error, "ERROR: EREMOTEIO: Remote I/O error");
                break;
            case -EDQUOT:
                __write_error_message(error, "ERROR: EDQUOT: Disk quota exceeded");
                break;
            case -ENOMEDIUM:
                __write_error_message(error, "ERROR: ENOMEDIUM: No medium found");
                break;
            case -EMEDIUMTYPE:
                __write_error_message(error, "ERROR: EMEDIUMTYPE: Wrong medium type");
                break;
            case -ECANCELED:
                __write_error_message(error, "WARN: ECANCELED: Operation canceled");
                break;
            case -ENOKEY:
                __write_error_message(error, "ERROR: ENOKEY: Required key not available");
                break;
            case -EKEYEXPIRED:
                __write_error_message(error, "ERROR: EKEYEXPIRED: Key has expired");
                break;
            case -EKEYREVOKED:
                __write_error_message(error, "ERROR: EKEYREVOKED: Key has been revoked");
                break;
            case -EKEYREJECTED:
                __write_error_message(error, "ERROR: EKEYREJECTED: Key was rejected by service");
                break;
            case -EOWNERDEAD:
                __write_error_message(error, "ERROR: EOWNERDEAD: Owner died");
                break;
            case -ENOTRECOVERABLE:
                __write_error_message(error, "ERROR: ENOTRECOVERABLE: State not recoverable");
                break;
            case -ERFKILL:
                __write_error_message(error, "ERROR: ERFKILL: Operation not possible due to RF-kill");
                break;
            case -EHWPOISON:
                __write_error_message(error, "ERROR: EHWPOISON: Memory page has hardware error");
                break;
            default:
                __write_error_message(error, "ERROR: Unknown error code");
                break;
        }
    }
    if (error->mode == ERROR_SAVE_ONLY) {
        return;
    }
    if (error->mode == ERROR_SAVE_AND_PRINT_TO_LOG || error->mode == ERROR_SAVE_AND_BLOCK) {
        l_output_error_message(error,NULL,0,printk);
    }
    if (error->mode == ERROR_SAVE_AND_BLOCK) 
    {
        while (1);
    }
    return;
}


int l_output_error_message(error_t *error,char *buffer,int buffer_size,void (out_function)(char *,...)) 
{
    if(error == NULL) return -1;
    if(buffer != NULL)
    {
        uint32_t sec  = error->time / HZ;
        uint32_t nsec = error->time % HZ;
        int len = snprintf(buffer, buffer_size, "(%u.%06u) (%d)[ \"%s\":(%d)] %s free memory size:%db\n",sec ,nsec,error->error_count, error->name, error->line,error->errmsg,error->haper_size);
        if(len >= error->errmsg_size) return -1;
        return len;
    }
    if(out_function != NULL)
    {
        uint32_t sec  = error->time / HZ;
        uint32_t nsec = error->time % HZ;
        out_function(KERN_INFO "(%u.%06u) (%d)[ \"%s\":(%d)] %s free memory size:%db\n",sec ,nsec,error->error_count, error->name, error->line,error->errmsg,error->haper_size);
    }
}