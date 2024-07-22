#ifndef UTILITY_H
#define UTILITY_H

//--------//

typedef struct linked_list_node {
    struct linked_list_node* next;
    void* value;
} linked_list_node;

linked_list_node* init_node(void* value);

typedef struct linked_list {
    struct linked_list_node* head;
    unsigned size;
} linked_list;

linked_list* init_linked_list();

void linked_list_free(linked_list* list);

linked_list_node* linked_list_get(linked_list* list, unsigned index);

void linked_list_append(linked_list* list, void* value);

void linked_list_insert(linked_list* list, void* value, unsigned index);

void linked_list_destroy(linked_list* list, unsigned index);

//--------//

void error(char* message, ...);

char* read_file(const char* path);

void write_file(const char* path, char* value);

char* mk_char_ptr(const char* str);

void str_append(char** str, char* value, unsigned extra_bytes);

char* str_fmt(char* fmt, unsigned extra_bytes, ...);

#endif