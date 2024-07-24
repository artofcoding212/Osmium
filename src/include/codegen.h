#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

typedef struct rt_var {
    unsigned long loc;
    char* name;
} rt_var;

rt_var* init_rt_var(char* name, unsigned long loc);

typedef struct rt_func {
    char* name;
    linked_list* params;
} rt_func;

rt_func* init_rt_func(char* name, linked_list* params);

unsigned long rt_func_param_size(rt_func* func);

typedef struct rt_scope {
    linked_list* vars;
    linked_list* funcs;
    struct rt_scope* parent;
    unsigned short is_main;
    long end_label;
} rt_scope;

rt_scope* init_rt_scope(rt_scope* parent, unsigned short is_main);

rt_var* rt_scope_lookup(rt_scope* scope, char* name);

rt_func* rt_scope_lookup_func(rt_scope* scope, char* name);

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