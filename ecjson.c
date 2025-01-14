#include <stdio.h>
#include <stdlib.h>   //内存分配
#include <string.h>  //计算字符串长度
#include <ctype.h>  //判断是否是数字等
#include "easy_cjson.h"


//注:请使用gcc编译器编译，其它可能会出错 (本代码是在wsl2配合vscode下编写的)
    //使用布尔值和NULL时暂时只支持小写



/*最后说一下这个解析器目前的功能：
将同目录下的json打开，读取里面的json数据（函数read_file_to_string)，然后清除里面的空白字符（这个函数在helper_functions里）
然后通过解析入口函数（json_parse）就能解析相应json数据*/

/*示例
 const char *json_text = read_file_to_string("example.json");  //读取当前目录中的example.json文件内容
    json_text = clean_json_whitespace(json_text); //将json_text指向一个已经清除无关字符的字符串（一定要这样,因为我没在其它函数里跳过空白字符)

    json_value *parsed = NULL;
    int result = json_parse(json_text, &parsed);  //调用json_parse函数将解析的结果存到json_value结构体中
    
    接下来就可以用C语言操作里面的数据，操作完后可以用print_json_value将操作完后的数据重新变成json格式
    */






//字符串解析函数（未考虑转义字符）
int parse_string(const char **json, char **out_str)  
{
    char *str = NULL;
    size_t len = 0;
    size_t capacity = 16; //初始分配的空间

    if (**json != '"')  //跳过开头的双引号
    {
        return -1;
    }
    (*json)++; //跳过双引号
    
    
    str = (char *)malloc(capacity);  //分配内存来存储字符串

    if (str == NULL) 
    {
        return -2;
    }

     while (**json != '"' && **json != '\0') 
     {
        str[len++] = **json; //复制到输出字符串
        (*json)++; // 移动到下一个字符
     

      if (len >= capacity) 
      {
            capacity *= 2;  //扩展两倍内存空间
            str = (char *)realloc(str, capacity);
            if (str == NULL) 
            {
                return -2;
            }
      }
     }

      
    if (**json == '"')  //检查是否遇到结束的双引号
    {
        str[len] = '\0';   //终止字符串
        *out_str = str;   //返回解析后的字符串
        (*json)++;   //跳过结束的双引号
        return JSON_OK;
    }

    //如果没有遇到结束的双引号，表示字符串不合法
    free(str);
    return JSON_INVALID_STRING;
}





//数字解析函数（不支持科学计数法和指数）
int parse_number(const char **json, double *out_num) 
{
    
    double result = 0.0;
    int sign = 1;  // 默认正号

    // 处理负号（如果有）
    if (**json == '-') 
    {
        sign = -1;
        (*json)++;  // 跳过负号
    }

    // 解析整数部分
    if (!isdigit(**json)) 
    {
        return JSON_INVALID_NUMBER;  
    }

    while (isdigit(**json)) 
    {
        result = result * 10 + (**json - '0');  // 将字符转化为数字
        (*json)++;  // 移动到下一个字符
    }

    // 解析小数部分（如果有）
    if (**json == '.') 
    {
        (*json)++;  // 跳过小数点
        double fraction = 1.0;
        if (!isdigit(**json)) 
        {
            return JSON_INVALID_NUMBER;  // 小数点后无数字则报错
        }
        while (isdigit(**json)) //fraction为分数
        {
            fraction /= 10.0;  // 每处理一位小数，分母就除以10
            result += (**json - '0') * fraction;
            (*json)++;  // 移动到下一个字符
        }
    }

    //指数部分暂时没有做


     //返回解析的数字，考虑符号
    *out_num = sign * result;
    return JSON_OK;

 }


// 解析布尔值 true
int parse_true(const char **json, int *out_bool) 
{
    if (strncmp(*json, "true", 4) == 0) //比较 *json 开始的 4 个字符是否和 "true" 完全匹配
    {
        *out_bool = 1; //1为真对应TRUE
        *json += 4;
        return JSON_OK;
    }
    return JSON_INVALID_BOOLEAN;
}

// 解析布尔值 false
int parse_false(const char **json, int *out_bool) 
{
    if (strncmp(*json, "false", 5) == 0) //比较 *json 开始的 4 个字符是否和 "false" 完全匹配
    {
        *out_bool = 0;  //c语言0为假
        *json += 5;
        return JSON_OK;
    }
    return JSON_INVALID_BOOLEAN;
}

// 解析 null
int parse_null(const char **json, int *out_null) 
{
    if (strncmp(*json, "null", 4) == 0) //比较 *json 开始的 4 个字符是否和 "false" 完全匹配
    {
        *out_null = 1; //   只是随便标记一下，如果成功了返回json_ok,类型直接被下面的值解析函数标记为null了
        *json += 4;
        return JSON_OK;
    }
    return JSON_INVALID_NULL;
}

// 解析值
int parse_value(const char **json, json_value **out_value) 
{
    char c = **json;
    *out_value = malloc(sizeof(json_value));
    if (!*out_value) 
    {
        return JSON_MEMORY_ERROR;
    }
//判断值的类型，根据值的类型调用相关解析函数

    if (c == '"')  //字符型
    {
        (*out_value)->type = JSON_TYPE_STRING;
        int result = parse_string(json, &(*out_value)->u.string); //将解析的结果存到json_value结构体中(下同)
        if (result != JSON_OK) 
        {
            free(*out_value);
            return result;
        }
    }
    else if (c == '-' || isdigit(c)) //数字型
    {
        (*out_value)->type = JSON_TYPE_NUMBER;
        int result = parse_number(json, &(*out_value)->u.number);
        if (result != JSON_OK) 
        {
            free(*out_value);
            return result;
        }
    }
    else if (c == '{') //对象
    {
        (*out_value)->type = JSON_TYPE_OBJECT;
        int result = parse_object(json, &(*out_value)->u.object, &(*out_value)->size);
        if (result != JSON_OK) 
        {
            free(*out_value);
            return result;
        }
    }
    else if (c == '[') //数组
    {
        (*out_value)->type = JSON_TYPE_ARRAY;
        int result = parse_array(json, &(*out_value)->u.array, &(*out_value)->size); //在结构体中存储值
        if (result != JSON_OK) 
        {
            free(*out_value);
            return result;
        }
    }
    else if (c == 't') //true
    {
        (*out_value)->type = JSON_TYPE_BOOLEAN;
        int bool_val;
        int result = parse_true(json, &bool_val);
        if (result != JSON_OK) 
        {
            free(*out_value);
            return result;
        }
        (*out_value)->u.boolean = bool_val;
    }
    else if (c == 'f') //flase
    {
        (*out_value)->type = JSON_TYPE_BOOLEAN;
        int bool_val;
        int result = parse_false(json, &bool_val);
        if (result != JSON_OK) 
        {
            free(*out_value);
            return result;
        }
        (*out_value)->u.boolean = bool_val;
    }
    else if (c == 'n') //null
    {
        (*out_value)->type = JSON_TYPE_NULL;
        int null_val;
        int result = parse_null(json, &null_val);
        if (result != JSON_OK) 
        {
            free(*out_value);
            return result;
        }
        // `null_val` 可以忽略，因为类型已经标识为 `JSON_TYPE_NULL`
    }
    else 
    {
        free(*out_value);
        return JSON_INVALID_OBJECT;  // 不支持的类型
    }

    return JSON_OK;
}

// 解析对象
int parse_object(const char **json, json_pair **out_obj, size_t *out_size) 
{
    // 跳过 '{'
    if (**json != '{') 
    {
        return JSON_INVALID_OBJECT;
    }
    (*json)++;

    json_pair *obj = NULL;
    size_t size = 0;
    size_t capacity = 8;
    obj = malloc(capacity * sizeof(json_pair));
    if (!obj) 
    {
        return JSON_MEMORY_ERROR;
    }

    while (**json != '}' && **json != '\0') //一个较大的循环，用来处理多个键值对
    {
        if (**json == '}') 
        {
            break;
        }

        if (**json != '"') 
        {
            // 释放已分配的键值对
            for (size_t i = 0; i < size; i++) {
                free(obj[i].key);
                free_json_value(obj[i].value);
            }
            free(obj);
            return JSON_INVALID_OBJECT;
        }

        // 解析键
        char *key = NULL;
        int result = parse_string(json, &key);
        if (result != JSON_OK) {
            // 解析失败，释放已分配的键值对
            for (size_t i = 0; i < size; i++) 
            {
                free(obj[i].key);
                free_json_value(obj[i].value);
            }
            free(obj);
            return result;
        }

        if (**json != ':') 
        {
            // 如果解析键值对缺少冒号
            free(key);
            for (size_t i = 0; i < size; i++) 
            {
                free(obj[i].key);
                free_json_value(obj[i].value);
            }
            free(obj);
            return JSON_INVALID_OBJECT;
        }
        (*json)++; // 跳过 ':'

        // 解析值
        json_value *value = NULL;
        result = parse_value(json, &value);
        if (result != JSON_OK) 
        {
            free(key);
            for (size_t i = 0; i < size; i++) 
            {
                free(obj[i].key);
                free_json_value(obj[i].value);
            }
            free(obj);
            return result;
        }

        // 添加到对象
        if (size >= capacity) 
        {
            capacity *= 2;
            json_pair *temp = realloc(obj, capacity * sizeof(json_pair));
            if (!temp) 
            {
                free(key);
                free_json_value(value);
                for (size_t i = 0; i < size; i++) 
                {
                    free(obj[i].key);
                    free_json_value(obj[i].value);
                }
                free(obj);
                return JSON_MEMORY_ERROR;
            }
            obj = temp;
        }
        obj[size].key = key;
        obj[size].value = value;
        size++; //每次多一个键值对，size的值加1

        // 跳过逗号
        if (**json == ',') 
        {
            (*json)++;
        }
    }//键值对循环结束

    // 跳过 '}'
    if (**json == '}') 
    {
        (*json)++;
        *out_obj = obj;
        *out_size = size;
        return JSON_OK;
    }

    // 如果没有遇到 '}', 解析失败
    for (size_t i = 0; i < size; i++) 
    {
        free(obj[i].key);
        free_json_value(obj[i].value);
    }
    free(obj);
    return JSON_INVALID_OBJECT;
}

// 解析数组
int parse_array(const char **json, json_value **out_array, size_t *out_size) 
{
    // 跳过 '['
    if (**json != '[') 
    {
        return JSON_INVALID_ARRAY;
    }
    (*json)++;

    // 初始化数组
    size_t capacity = 8;
    size_t size = 0;
    json_value *array = malloc(capacity * sizeof(json_value));
    if (!array) 
    {
        return JSON_MEMORY_ERROR;
    }

    // 解析数组元素
    while (**json != ']' && **json != '\0') 
    {
        // 解析值
        json_value *value = NULL;
        int result = parse_value(json, &value);  //value是一个json_value结构体的指针
        if (result != JSON_OK) 
        {
            // 解析失败，释放已分配的内存
            for (size_t i = 0; i < size; i++) 
            {
                free_json_value(&array[i]);
            }
            free(array);
            return result;
        }

        // 添加到数组
        if (size >= capacity) 
        {
            capacity *= 2;
            json_value *temp = realloc(array, capacity * sizeof(json_value));
            if (!temp) {
                // 重新分配失败，释放已分配的内存
                for (size_t i = 0; i < size; i++) 
                {
                    free_json_value(&array[i]);
                }
                free(array);
                return JSON_MEMORY_ERROR;
            }
            array = temp;
        }
        array[size++] = *value; // *value是json_value结构体,array数组中存储了一个又一个结构体，这样就可以实现数组里存储不同类型的元素，要想访问这些元素，得通过里面的type这个结构体成员来确定元素的类型，再访问联合体中的相应元素，从而正确打印
        free(value); // 因为 parse_value 已经分配了一个 json_value，并赋值到数组中，复制后可以释放

        // 跳过逗号
        if (**json == ',') 
        {
            (*json)++;
        }
    }

    // 跳过 ']'
    if (**json == ']') 
    {
        (*json)++;
        *out_array = array;
        *out_size = size;
        return JSON_OK;
    }

    // 如果没有遇到 ']', 解析失败
    for (size_t i = 0; i < size; i++) 
    {
        free_json_value(&array[i]);
    }
    free(array);
    return JSON_INVALID_ARRAY;
}

// 释放 JSON 值
void free_json_value(json_value *val) 
{
    if (!val) return;

    switch (val->type) {
        case JSON_TYPE_STRING:
            free(val->u.string);
            break;
        case JSON_TYPE_OBJECT: {
            json_pair *pairs = val->u.object;
            for (size_t i = 0; i < val->size; i++) 
            {
                free(pairs[i].key);
                free_json_value(pairs[i].value);
            }
            free(pairs);
            break;
        }
        case JSON_TYPE_ARRAY: 
        {
            json_value *array = val->u.array;
            for (size_t i = 0; i < val->size; i++) 
            {
                free_json_value(&array[i]);
            }
            free(array);
            break;
        }
        case JSON_TYPE_NULL:
        case JSON_TYPE_BOOLEAN:
        case JSON_TYPE_NUMBER:
            // 无需释放(常规类型)
            break;
    }
    //free(val);         注意val没被释放，但是释放会报错，可能会导致内存泄漏，暂时没有解决，
}






// 打印 JSON 值（递归）
void print_json_value(const json_value *val, int indent) //indent用来缩进(没嵌套一次多缩进一次，使层次分明)
{
    if (!val) return;
    for (int i = 0; i < indent; i++) printf("  "); // 缩进
    switch (val->type) {
        case JSON_TYPE_NULL:
            printf("null\n");
            break;
        case JSON_TYPE_BOOLEAN:
            printf(val->u.boolean ? "true\n" : "false\n");
            break;
        case JSON_TYPE_NUMBER:
            printf("%lf\n", val->u.number);
            break;
        case JSON_TYPE_STRING:
            printf("\"%s\"\n", val->u.string);
            break;
        case JSON_TYPE_OBJECT: 
        {
            printf("{\n");
            for (size_t i = 0; i < val->size; i++)                         //有几个键值对就打印几次
            {
                for (int j = 0; j < indent + 1; j++) printf("  ");
                printf("\"%s\": ", val->u.object[i].key);                 //打印出键
                print_json_value(val->u.object[i].value, indent + 1);     //值有可能是嵌套的，进入递归知道最后一层不能嵌套（字符数字布尔类型）
            }
            for (int i = 0; i < indent; i++) printf("  ");
            printf("}\n");
            break;
        }
        case JSON_TYPE_ARRAY: 
        {
            printf("[\n");
            for (size_t i = 0; i < val->size; i++) 
            {
                print_json_value(&val->u.array[i], indent + 1);       //递归
            }
            for (int i = 0; i < indent; i++) printf("  ");
            printf("]\n");
            break;
        }
    }
}





// 总解析函数
int json_parse(const char *json_str, json_value **out_val) 
{
    const char *p = json_str;

    int result = parse_value(&p, out_val);
    if (result != JSON_OK) 
    {
        return result;
    }

    // 确保整个字符串被解析完毕
    if (*p != '\0') 
    {
        free_json_value(*out_val);
        *out_val = NULL;
        return JSON_INVALID_OBJECT;
    }

    return JSON_OK;
}






//如果要把数据存到另外的结构体可以参考这个函数


/*int json_to_student(const json_value *val, Student *student) 
{
    if (val->type != JSON_TYPE_OBJECT) return -1;

    memset(student, 0, sizeof(Student));

    for (size_t i = 0; i < val->size; i++) 
    {
        json_pair pair = val->u.object[i];
        if (strcmp(pair.key, "name") == 0 && pair.value->type == JSON_TYPE_STRING) 
        {
            student->name = strdup(pair.value->u.string);
        } else if (strcmp(pair.key, "age") == 0 && pair.value->type == JSON_TYPE_NUMBER) 
        {
            student->age = (int)pair.value->u.number;
        } else if (strcmp(pair.key, "is_student") == 0 && pair.value->type == JSON_TYPE_BOOLEAN) 
        {
            student->is_student = pair.value->u.boolean;
        } else if (strcmp(pair.key, "scores") == 0 && pair.value->type == JSON_TYPE_ARRAY) 
        {
            student->scores_count = pair.value->size;
            student->scores = malloc(student->scores_count * sizeof(double));
            if (!student->scores) return -2;
            for (size_t j = 0; j < student->scores_count; j++) 
            {
                if (pair.value->u.array[j].type == JSON_TYPE_NUMBER) 
                {
                    student->scores[j] = pair.value->u.array[j].u.number;
                } else 
                {
                    free(student->scores);
                    return -3;
                }
            }
        }
        // 处理其他字段...
    }
    return 0;
}*/





