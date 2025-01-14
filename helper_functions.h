#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_h

int is_whitespace(char c);
char* clean_json_whitespace(const char *json_str);
char* read_file_to_string(const char *filename);

#endif