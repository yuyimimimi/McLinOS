#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>


#define KEY_VALUE_MAX_LENGTH 64

static char* goto_next_char(char* data,char c,char limit)
{
    char *index = data;
    while(1){
        if(index[0] == '\0')
            return NULL;
        if(index[0] == limit)
            return index;
        if(index[0] == c){
          return index;            
        }
        else
        index++;
    }
}
char* __goto_next_char(char* data,char c,char limit)
{
    return goto_next_char(data,c, limit);
}
static char* goto_next_string(char* data,char limit)
{
    char *index = data;
    while(1)
    {
        if(index[0] == '\0'){
            return NULL;
        }
        if(index[0] == limit){
           return NULL;            
        }

        if ((index[0] >= '0' && index[0] <='9') 
          ||(index[0] >= 'a' && index[0] <='z') 
          ||(index[0] >= 'A' && index[0] <='Z') 
          ||(index[0] == '_')
          ||(index[0] == '\"' )
          ||(index[0] == '/' )
        )
        return index;
        else
        index++;
    }   
}
char* __goto_next_string(char* data,char limit){
    return goto_next_string( data, limit);
}

static int copy_string_data(char* from,char* to)
{
    char* index = from;
    int i=0;
    
    if(index[0] == '\"')
    {
        index++;
        for(;
            index[i] != '\0' && 
            index[i] != '\n' && 
            index[i] != '\"'
        ;i++)
        {
            to[i] = index[i];
        }
        to[i] ='\0';
        i+=2;
        return i;   
    }
    else
    {
        for(;(
            index[i] >= '0' && index[i] <='9') 
        ||(index[i] >= 'a' && index[i] <='z') 
        ||(index[i] >= 'A' && index[i] <='Z') 
        ||(index[i] > '!' && index[i] <= '/' ) 
        ||(index[i] > ':' && index[i] <= '@' )
        ||(index[i] == '_')

        ;i++)
        {
            to[i] = index[i];
        }  
        to[i] ='\0';
        //pr_info("to: %s\n",to);
        return i;      
    }
}

int __copy_string_data(char* from,char* to)
{
    return copy_string_data(from, to);
}

static char* get_next_head(char *text_data,char *buffer)
{
    char *index = text_data;
    index = goto_next_char(index,'[',']');
    if(index == NULL) return NULL;
    index++;
    index = goto_next_string(index,']');
    if(index == NULL) return NULL;
    int length = copy_string_data(index,buffer);
    index +=length;
    index = goto_next_char(index,']','\0');
    if(index == NULL) return NULL;
    char* new_index;
    new_index = goto_next_char(index,'\n','\0');
    if(new_index == NULL) return index;
    else             return new_index;
}



static char* get_next_key(char *text_data,char *buffer)
{
    char *index = text_data;
    index = goto_next_string(index,'[');
    if(index == NULL){
         return NULL;
    }
    int length = copy_string_data(index,buffer);
    index +=length;  
    return index;
}

static char* get_key(char *text_data,char *data)
{
    char key_data[32]; 
    key_data[31] = '\0';
    char *index = text_data;
    while (1)
    {
        index = get_next_key(index,key_data);
        if(index == NULL)
        break;
        if(strcmp(data,key_data)==0){
            return index;
        }
        index = goto_next_char(index,'\n','\0');
        if(index == NULL)
        break;
        index++;
    }
    return NULL;
}
static char* get_value(char *text_data,char *key,char* value)
{
    char* index = text_data;
    index =get_key(index,key);
    if(index == NULL) return NULL;
    index = goto_next_char(index,'=','\n');
    index++;
    index = get_next_key(index,value);
    return index;
}

static char* get_head_start(char* text_data,char* head)
{
    char *index = text_data;
    char buffer[KEY_VALUE_MAX_LENGTH];
    while (1)
    {
        index = get_next_head(index,buffer);           
        if(index == NULL)
        {
            return NULL;
        }
        if(strcmp(buffer,head) == 0)
        {
            return index;
        }
        // else
        // {
        //     pr_info("needhead:%s,headdata:%s\n",head,buffer);
        // }
    }
}

char* get_value_from_ini(char *text_data,char *grou,char* key,char* value)
{
    if(text_data == NULL  || key == NULL || value == NULL) 
    return NULL;
    
    char* index = text_data;
    if(grou != NULL){
        index = get_head_start(index,grou); //获取起始头部
        if(index == NULL){
            pr_err("can not get this file head\n");
            return NULL;               
        }
    }
    index = get_value(index,key,value);  
    if(index == NULL){
        pr_err("can not get value\n");    
    }
    return index;
}

void preprocess_ini_data(char *data)
{
    char *src = data;
    char *dst = data;
    while (*src != '\0') 
    {
        if (*src == '#' || *src == ';' ) {
            while (*src != '\0' && *src != '\n') {
                src++;
            }
        } 
        else {
            *dst++ = *src++;
        }
    }
    *dst = '\0'; 
}

