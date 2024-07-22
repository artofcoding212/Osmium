#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "utility.h"

typedef enum token_type {
    //--LITERALS--//
    TOKEN_IDENT,
    TOKEN_INT,
    TOKEN_STR,
    //--GROUPINGS--//
    TOKEN_LBRACK,
    TOKEN_RBRACK,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    //--OPERATORS--//
    TOKEN_SEMI,
    TOKEN_COMMA,
    TOKEN_EQ,
    TOKEN_RARROW,
    TOKEN_COLON,
    //--MATHEMATICS--//
    TOKEN_GREAT,
    TOKEN_GT,
    TOKEN_LESS,
    TOKEN_LT,
    TOKEN_EQUIV,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    //--SYSTEM--//
    TOKEN_EOF,
} token_type;

typedef struct token {
    unsigned long line;
    token_type type;
    char* value;
} token;

token* init_token(token_type type, unsigned long line, char* value);

linked_list* tokenize(char* src);

#endif