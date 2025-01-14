#ifndef EASY_CJSON_H
#define EASY_CJSON_H

// 错误代码
#define JSON_OK 0
#define JSON_INVALID_STRING 100
#define JSON_MEMORY_ERROR 200
#define JSON_INVALID_NUMBER 300
#define JSON_INVALID_OBJECT 400
#define JSON_INVALID_ARRAY 500
#define JSON_INVALID_BOOLEAN 600
#define JSON_INVALID_NULL 700


/*typedef struct {
    char *name;
    int age;
    int is_student;
    double *scores;
    size_t scores_count;
} Student;*/


// 定义 JSON 值类型的枚举
typedef enum {
    JSON_TYPE_NULL,      //0
    JSON_TYPE_BOOLEAN,   //1
    JSON_TYPE_NUMBER,    //2
    JSON_TYPE_STRING,    //3
    JSON_TYPE_OBJECT,    //4
    JSON_TYPE_ARRAY      //5
} JSON_VALUE_TYPE;

// 声明 json_value 结构体，因为 json_pair 需要用到它
typedef struct json_value json_value;

// 键值对结构
typedef struct {
    char *key;
    json_value *value;  // 指向 json_value结构体的指针
} json_pair;

// 定义 JSON 值结构体
struct json_value {
    JSON_VALUE_TYPE type;  // JSON 值的类型
    size_t size;           // 对象的键值对数量或数组的元素数量
    union {   //联合体，共享内存，一个此结构体同时只能存在一个类型
        int boolean;        // 布尔值(未考虑true和false的大小写问题)
        double number;      // 数字（可以是整数或浮点数,没有做科学计数法和指数功能）
        char *string;       // 字符串（以字符指针形式存储，暂时没有判断转义字符的功能）
        json_pair *object;  // 对象（键值对数组）
        json_value *array;  // 数组（数组元素）
    } u;  // 联合体成员的名称
};

// 函数声明
//int json_to_student(const json_value *val, Student *student);
int parse_string(const char **json, char **out_str);
int parse_number(const char **json, double *out_num);
int parse_true(const char **json, int *out_bool);
int parse_false(const char **json, int *out_bool);
int parse_null(const char **json, int *out_null);
int parse_object(const char **json, json_pair **out_obj, size_t *out_size);
int parse_array(const char **json, json_value **out_array, size_t *out_size);
int parse_value(const char **json, json_value **out_value);
void free_json_value(json_value *val);
void print_json_value(const json_value *val, int indent);
int json_parse(const char *json_str, json_value **out_val);



#endif