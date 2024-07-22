#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

typedef struct rt_var {
    unsigned long loc;
    char* name;
} rt_var;

rt_var* init_rt_var(char* name, unsigned long loc);

typedef struct rt_scope {
    linked_list* vars;
    struct rt_scope* parent;
    unsigned short is_main;
} rt_scope;

rt_scope* init_rt_scope(rt_scope* parent, unsigned short is_main);

rt_var* rt_scope_lookup(rt_scope* scope, char* name);

typedef struct codegen {
    char* result;
    rt_scope* main_scope;
    linked_list* tree;
    linked_list* scopes;
    unsigned long stack_size;
    unsigned long label_count;
} codegen;

codegen* init_codegen(linked_list* tree);

char* codegen_gen(codegen* codegen);

#endif