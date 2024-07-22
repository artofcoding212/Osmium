#include "include/utility.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

linked_list_node* init_node(void* value){
    linked_list_node* node = calloc(1, sizeof(struct linked_list_node));
    node->next = 0;
    node->value = value;

    return node;
}

linked_list* init_linked_list(){
    linked_list* list = calloc(1, sizeof(struct linked_list));
    list->head = 0;
    list->size = 0;

    return list;
}

void linked_list_free(linked_list* list){
    while(list->head)
        linked_list_destroy(list, 0);
}

linked_list_node* linked_list_get(linked_list* list, unsigned index){
    unsigned current_index = 0;
    linked_list_node* current_node = list->head;

    while(current_node > 0 && current_index < index){
        current_node = current_node->next;
        current_index++;
    }

    return current_node;
}

void linked_list_append(linked_list* list, void* value){
    linked_list_insert(list, value, list->size);
}

void linked_list_insert(linked_list* list, void* value, unsigned index){
    if(index > list->size)
        return;

    linked_list_node* new_node = init_node(value);

    if(index == 0){
        new_node->next = list->head;
        list->head = new_node;
    }else{
        linked_list_node* node_at_index = linked_list_get(list, index-1);
        if(!node_at_index){
            free(new_node);
            return;
        }

        new_node->next = node_at_index->next;
        node_at_index->next = new_node;
    }

    list->size++;
}

void linked_list_destroy(linked_list* list, unsigned index){
    if(index >= list->size)
        return;

    linked_list_node* target_node;

    if(index == 0){
        target_node = list->head;
        list->head = target_node->next;
    }else{
        linked_list_node* node_at_index = linked_list_get(list, index-1);
        if(!node_at_index || !node_at_index->next)
            return;

        target_node = node_at_index->next;
        node_at_index->next = target_node->next;
    }

    free(target_node);
    list->size--;
}

void error(char* message, ...){
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);

    exit(1);
}

char* read_file(const char* path){
    FILE* file = fopen(path, "rb");
    if(file == NULL)
        error("[utility]-> couldn't read file \"%s\"\n", path);

    unsigned long length = 0;
    char* line = NULL;
    long read;

    char* buffer = calloc(1, sizeof(char));
    buffer[0] = '\0';

    while((read = getline(&line, &length, file)) != -1){
        buffer = realloc(buffer, (strlen(buffer) + strlen(line) + 1) * sizeof(char));
        strcat(buffer, line);
    }

    fclose(file);
    if(line)
        free(line);

    return buffer;
}

void write_file(const char* path, char* value){
    FILE* file = fopen(path, "w");
    if(file == NULL)
        error("[utility]-> couldn't open file \"%s\" to write to\n", path);
    fprintf(file, "%s", value);
    fclose(file);
}

char* mk_char_ptr(const char* str){
    char* ptr = calloc(strlen(str + 2), sizeof(char));
    ptr = (char*)str;

    return ptr;
}

void str_append(char** str, char* value, unsigned extra_bytes){
    *str = realloc(*str, (strlen(*str) + strlen(value) + extra_bytes) * sizeof(char));
    strcat(*str, value);
}

char* str_fmt(char* fmt, unsigned extra_bytes, ...){
    va_list args;
    va_start(args, extra_bytes);
    int len = vsnprintf(NULL, 0, fmt, args) + extra_bytes + 1;
    va_end(args);

    char* str = malloc(len);

    va_start(args, extra_bytes);
    vsnprintf(str, len, fmt, args);
    va_end(args);
    return str;
}