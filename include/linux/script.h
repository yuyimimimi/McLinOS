#ifndef  __SCRIPT_H_
#define  __SCRIPT_H_

extern char* get_value_from_ini(char *text_data,char *grou,char* key,char* value); //获取ini格式数据
extern void preprocess_ini_data(char *data);                                       //通用数据预处理，用于处理#注释;注释


extern char* __goto_next_string(char* data,char limit);
static char*  goto_next_string(char* data,char limit){
  return __goto_next_string(data,limit);
}

extern int __copy_string_data(char* from,char* to);
static int copy_string_data(char* from,char* to){
   return __copy_string_data(from,to);
}

extern char* __goto_next_char(char* data,char c,char limit);
static char* goto_next_char(char* data,char c,char limit){
    return __goto_next_char(data,c, limit);
}




#endif // ! 