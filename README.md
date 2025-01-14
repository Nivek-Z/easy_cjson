# easy_cjson

这是一个简易的C语言json解析器


主要函数在ecjson.c中，helper_functions.c中只有清除空白字符和读取文件内容的函数


/*功能示例
在主函数中如下操作

    const char *json_text = read_file_to_string("example.json");  //读取当前目录中的example.json文件内容

    json_text = clean_json_whitespace(json_text); //将json_text指向一个已经清除无关字符的字符串（一定要这样,因为我没在其它函数里跳过空白字符)

    json_value *parsed = NULL;

    int result = json_parse(json_text, &parsed);  //调用json_parse函数将解析的结果存到json_value结构体中
    
    接下来就可以用C语言操作里面的数据，操作完后可以用print_json_value将操作完后的数据重新变成json格式
*/