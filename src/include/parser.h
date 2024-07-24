#ifndef PARSER_H
#define PARSER_H

#include "utility.h"
#include "tokenizer.h"

typedef enum ast_type {
    //--STATEMENTS--//
    AST_FUNC_DEF,
    AST_VAR_DEF,
    AST_RET,
    AST_SCOPE,
    AST_IF_TREE,
    AST_LOOP,
    AST_BRK,
    AST_CNT,
    //--EXPRESSIONS--//
    AST_IDENT,
    AST_INT,
    AST_STR,
    AST_BIN,
    AST_CALL,
    //--TYPES--//
    AST_IDENT_T,
    AST_LIT_T,
} ast_type;

typedef struct ast_node {
    ast_type type;
    unsigned long line;

    union{
        struct {
            char* name;
            struct ast_node* ret_type;
            linked_list* params;
            linked_list* body;
        } func;
        struct {
            char* name;
            struct ast_node* value;
        } var;
        struct {
            struct ast_node* value;
        } ret;
        struct {
            linked_list* body;
        } scope;
        struct {
            struct ast_node* condition;
            struct ast_node* then_scope;
            struct ast_node* else_body;
        } if_tree;
        struct {
            struct ast_node* condition;
            struct ast_node* scope;
        } loop;
        struct {
            char* value;
        } ident;
        struct {
            unsigned long value;
        } integer;
        struct {
            char* value;
        } str;
        struct {
            struct ast_node* left;
            struct ast_node* right;
            char* op;
        } bin;
        struct{ 
            char* caller;
            linked_list* args;
        } call;
        struct {
            struct ast_node* lit;
        } lit_t;
    };
} ast_node;

ast_node* init_ast_node(ast_type type, unsigned long ln);

typedef enum parser_state {
    PARSER_FWD,
    PARSER_BWD,
} parser_state;

typedef struct parser {
    parser_state state;
    linked_list* tokens;

    token* token;
    unsigned pos;
} parser;

parser* init_parser(linked_list* tokens);

linked_list* parser_parse(parser* parser);

#endif