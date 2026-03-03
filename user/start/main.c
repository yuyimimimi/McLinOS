#include <stdio.h>
#include <fcntl.h>    
#include <unistd.h>    
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>


char data[] = "hello_test\n";

int main(int argc, char *argv[]) 
{
    int fp = open("/dev/ttyS0", O_RDWR); 
    write(fp,data,sizeof(data));
    int length = 0;

    char buffer[64];
    while(1)
    {
        length = read(fp,&buffer,1);
        if(length > 1)
        {
            write(fp,&buffer,length);
        }
    }
    close(fp);   
    return 0;
}

