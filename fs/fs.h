#ifndef _FS_H_
#define _FS_H_

#include <linux/fs.h>    

static int get_dir_name(const char *path, char *dir_name,int nummber) //获取指定目录中指定位置的目录名
{
    int i = 0,j = 0;
    while (path[i]!= '\0'){
        if(path[i] == '/')
        j++;
        if(j == nummber)
        goto a;
        i++;
    }
    return -1;
    a:
    i++;
    j = 0;
    while (path[i+j]!= '\0'){
       dir_name[j] = path[i+j];
       if(path[i+j] == '/')
       break;
       j++;
    }
    dir_name[j] = '\0';
    return 0;
}

static int get_dir_number(char *path){                       //获取指定目录的层级数
    int j = 0,i = 0;
    while (path[j]!= '\0'){
        if(path[j] == '/' && path[j+1] != '\0')
        i++;
        j++;
    }
    return i;
}

#endif 
