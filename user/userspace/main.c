#include <stdio.h>
#include <fcntl.h>    
#include <unistd.h>    
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

struct shell {
    char cmd_buffer[128]; 
    uint32_t buffer_size;
    uint32_t Cursor;
    int file;
    char buffer[128];
};



static int shell_init(struct shell* new,char* tty_dev_path){
    new->file = open(tty_dev_path, O_RDWR);
    if(new->file < 0){
        return -1;
    }
    new->buffer_size = 0;
    new->Cursor     = 0;
    return 0;
};
static void shell_exit(struct shell* new){
    close(new->file);   
}

static int read_char(struct shell* s ,char* c){
    return read(s->file,c,1);
}
static int write_char(struct shell* s ,char* text,int length)
{
    return write(s->file,text,length);
}


struct shell shell;
int main(int argc,char*argv[]) 
{
    int err =shell_init(&shell,"/dev/ttyS0");
    if(err < 0){
        return -1;
    }

    shell_exit(&shell);



//     int fp = open("/dev/ttyS0", O_RDWR); 
//     write(fp,test_data,strlen(test_data));

//     for(int i =0;i < argc;i++){
//         write(fp,argv[i],strlen(argv[i]));   
//         write(fp,"\n",1);        
//     }
//     close(fp);   
    return 0;
}
