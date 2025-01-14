#include <stdio.h>
#include <stdlib.h>   //内存分配
#include <string.h>  //计算字符串长度
#include <ctype.h>  //判断是否是数字等



int is_whitespace(char c) //判断是否是一些空白字符和一些无关字符，返回0为否，返回其它整数为真
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f' || c=='\\') ;
}



char* clean_json_whitespace(const char *json_str) 
{
    int len = strlen(json_str);  //计算输入字符串的长度
    char *result = (char *)malloc(len + 1); //为输出的字符串分配内存空间
    int index = 0;
    int i=0;

    for ( i = 0; i < len; i++)//遍历输入的字符串
    {
        if (is_whitespace(json_str[i])) //判断是否为无关字符，是的话跳过此次循环
        {
            continue;
        }
    result[index++] = json_str[i]; //跳过无关字符后，将有效字符赋给result字符数组  注意：暂时没有考虑有注释时候的情况
    
    }
    result[index] = '\0'; //在结尾加上结束标志
    return result; //返回result字符串（即去掉所有无关字符后的字符串）
}




//---------------------------------------------------------------------------------------------------//




char* read_file_to_string(const char *filename)  //打开并读取json文件
{
    FILE *file = fopen(filename, "r");  // 打开文件
    if (!file) {
        printf("Error: 无法打开该文件 %s\n", filename);
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);           // 将文件指针移动到文件末尾
    long file_size = ftell(file);       // 获取文件指针位置（即文件大小）
    rewind(file);                       // 将文件指针重置到文件开头

    
    char *content = (char *)malloc((file_size + 1) * sizeof(char));  // 分配内存以保存文件内容
    if (!content) {
        printf("Error: 内存分配失败\n");
        fclose(file);
        return NULL;
    }

    fread(content, sizeof(char), file_size, file);  // 读取文件内容到内存中
    content[file_size] = '\0';  // 确保字符串以 '\0' 结尾

    fclose(file);  // 关闭文件
    return content;
}
