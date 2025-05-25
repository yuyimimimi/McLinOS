#include <sys/types.h>    // 用于 mode_t, size_t, ssize_t 等类型
#include <fcntl.h>        // 用于 open() 的标志常量如 O_RDONLY 等

int __system_call(int sys_no, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5);

int _open(const char *pathname, int flags, mode_t mode) {
    return __system_call(5, (int)pathname, flags, mode,0,0,0);
}

ssize_t _write(int fd, const void *buf, size_t count) {
    return __system_call(4, fd, (int)buf, count,0,0,0);
}

ssize_t _read(int fd, void *buf, size_t count) {
    return __system_call(3, fd, (int)buf, count,0,0,0);
}

int _close(int fd) {
    return __system_call(6, fd,0,0,0,0,0);
}

int _getdents(int fd, void *dirp, unsigned int count) {
    return __system_call(78, fd, (int)dirp, count,0,0,0);
}
void _exit(){
    __system_call(1, 0,0,0,0,0,0);
}

void *_sbrk(ptrdiff_t incr) {
    return NULL;
}

off_t _lseek(int fd, off_t offset, int whence) {
    return (off_t)-1; 
}

void nanosleep(unsigned int time ){
    __system_call(162, time,0,0,0,0,0);
}


int _execve(const char *path, char * const argv[], char * const envp[]){
       return __system_call(11, path,argv,envp,0,0,0);
}
