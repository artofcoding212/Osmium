#include "include/tokenizer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

token* init_token(token_type type, unsigned long line, char* value){
    token* token = calloc(1, sizeof(struct token));
    token->type = type;
    token->value = value;
    token->line = line;

    return token;
}

linked_list* tokenize(char* src){
    unsigned long src_size = strlen(src);
    unsigned long line = 1;
    unsigned pos = 0;
    char current = src[pos];

    linked_list* tokens = init_linked_list();

    while(current != '\0'){
        while(current == 13 || current == 10 || current == '\t' || current == '/' || current == ' '){
            if(current == 13 || current == 10)
                line++;

            if(current == '/'){
                if(src[pos + 1] == '*'){
                    pos += 2;
                    current = src[pos];

                    while(current != '\0'){
                        if(current == '*' && src[pos + 1] == '/')
                            break;
                        if(current == 13 || current == 10)
                            line++;

                        pos++;
                        current = src[pos];
                    }

                    if(current != '*' || src[pos + 1] != '/')
                        error("[tokenizer]-> encountered unterminated multi-line comment\n");
                
                    continue;
                }

                if(src[pos + 1] != '/')
                    break;

                pos += 2;
                current = src[pos];

                while(current != 13 && current != 10 && current != '\0'){
                    pos++;
                    current = src[pos];
                }

                continue;
            }
            
            pos++;
            current = src[pos];
        }
        if(isalpha(current) || current == '_'){
            char* value = calloc(1, sizeof(char));

            while(isalnum(current) || current == '_'){
                value = realloc(value, (strlen(value) + 2) * sizeof(char));
                strcat(value, (char[]){current, 0});
                pos++;
                current = src[pos];
            }

            if(strncmp("__CORE_", value, strlen("__CORE_")) == 0)
                error("[tokenizer]-> identifiers cannot start with \"__CORE_\"\n");

            linked_list_append(tokens, init_token(TOKEN_IDENT, line, value));
            continue;
        }
        if(isdigit(current)){
            char* value = calloc(1, sizeof(char));

            while(isdigit(current)){
                value = realloc(value, (strlen(value) + 2) * sizeof(char));
                strcat(value, (char[]){current, 0});
                pos++;
                current = src[pos];
            }

            linked_list_append(tokens, init_token(TOKEN_INT, line, value));
            continue;
        }
        if(current == '"'){
            pos++;
            current = src[pos];
            char* value = calloc(1, sizeof(char));

            while(current != '\0' && current != '"'){
                value = realloc(value, (strlen(value) + 2) * sizeof(char));
                strcat(value, (char[]){current, 0});
                pos++;
                current = src[pos];
            }

            if(current != '"')
                error("[tokenizer]-> encountered unterminated string\n");
            pos++;
            current = src[pos];

            linked_list_append(tokens, init_token(TOKEN_STR, line, value));
            continue;
        }

        switch(current){
            case '[': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_LBRACK, line, mk_char_ptr("[")));
                continue;
            }
            case ']': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_RBRACK, line, mk_char_ptr("]")));
                continue;
            }
            case '(': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_LPAREN, line, mk_char_ptr("(")));
                continue;
            }
            case ')': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_RPAREN, line, mk_char_ptr(")")));
                continue;
            }
            case '{': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_LBRACE, line, mk_char_ptr("{")));
                continue;
            }
            case '}': {
                pos++;
                current = src[pos];
                
                linked_list_append(tokens, init_token(TOKEN_RBRACE, line, mk_char_ptr("}")));
                continue;
            }
            case ';': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_SEMI, line, mk_char_ptr(";")));
                continue;
            }
            case ',': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_COMMA, line, mk_char_ptr(",")));
                continue;
            }
            case '=': {
                pos++;
                current = src[pos];

                if(current == '='){
                    pos++;
                    current = src[pos];
                    linked_list_append(tokens, init_token(TOKEN_EQUIV, line, mk_char_ptr("==")));
                    continue;
                }

                linked_list_append(tokens, init_token(TOKEN_EQ, line, mk_char_ptr("=")));
                continue;
            }
            case ':': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_COLON, line, mk_char_ptr(":")));
                continue;
            }
            case '>': {
                pos++;
                current = src[pos];

                if(current == '='){
                    pos++;
                    current = src[pos];
                    linked_list_append(tokens, init_token(TOKEN_GT, line, mk_char_ptr(">=")));
                    continue;
                }

                linked_list_append(tokens, init_token(TOKEN_GREAT, line, mk_char_ptr(">")));
                continue;
            }
            case '<': {
                pos++;
                current = src[pos];

                if(current == '='){
                    pos++;
                    current = src[pos];
                    linked_list_append(tokens, init_token(TOKEN_LT, line, mk_char_ptr("<=")));
                    continue;
                }

                linked_list_append(tokens, init_token(TOKEN_LESS, line, mk_char_ptr("<")));
                continue;
            }
            case '-': {
                pos++;
                current = src[pos];

                if(current == '>'){
                    pos++;
                    current = src[pos];
                    linked_list_append(tokens, init_token(TOKEN_RARROW, line, mk_char_ptr("->")));
                    continue;
                }

                linked_list_append(tokens, init_token(TOKEN_MINUS, line, mk_char_ptr("-")));
                continue;
            }
            case '+': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_PLUS, line, mk_char_ptr("+")));
                continue;
            }
            case '*': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_STAR, line, mk_char_ptr("*")));
                continue;
            }
            case '/': {
                pos++;
                current = src[pos];

                linked_list_append(tokens, init_token(TOKEN_SLASH, line, mk_char_ptr("/")));
                continue;
            }
            case '\0': break;
            default: error("[tokenizer]-> unrecognized char '%c'\n", current);
        }
    }

    linked_list_append(tokens, init_token(TOKEN_EOF, line, 0));

    return tokens;
}