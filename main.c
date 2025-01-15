#include <stdio.h>
#include <stdlib.h>   //内存分配
#include <string.h>  //计算字符串长度
#include <ctype.h>  //判断是否是数字等
#include "easy_cjson.h"
#include "helper_functions.h"

// 主函数用于测试
int main() 
{
    // 确保你的 JSON 数据没有空白字符
    const char *json_text = read_file_to_string("example.json");
    //printf("%s",json_text);
    json_text = clean_json_whitespace(json_text);

    json_value *parsed = NULL;
    int result = json_parse(json_text, &parsed); 

    if (result == JSON_OK) 
    {
        printf("JSON parsed successfully!\n");
        print_json_value(parsed, 0);
        free_json_value(parsed); //释放结构体中的存储的指针
        free(parsed); //最后把结构体释放
    }
    else 
    {
        printf("Failed to parse JSON. Error code: %d\n", result);
    }

    return 0;
}