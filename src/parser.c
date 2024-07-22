#include "include/parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

ast_node* init_ast_node(ast_type type, unsigned long ln){
    ast_node* node = calloc(1, sizeof(struct ast_node));
    node->type = type;
    node->line = ln;

    return node;
}

parser* init_parser(linked_list* tokens){
    parser* parser = calloc(1, sizeof(struct parser));
    parser->state = PARSER_FWD;
    parser->tokens = tokens;
    parser->token = tokens->head->value;
    parser->pos = 0;

    return parser;
}

static token* parser_mov(parser* parser){
    switch(parser->state){
        case PARSER_FWD: {
            if(parser->pos >= parser->tokens->size - 1)
                return parser->token;

            token* passing = parser->token;

            parser->pos++;
            linked_list_node* node = linked_list_get(parser->tokens, parser->pos);
            parser->token = node->value;
            return passing;
        }
        case PARSER_BWD: {
            if(parser->pos <= 0)
                return parser->token;

            token* passing = parser->token;

            parser->pos--;
            parser->token = linked_list_get(parser->tokens, parser->pos)->value;

            return passing;
        }
        default: error("[parser]-> unrecognized state '%d'\n", parser->state);
    }
}

static token* parser_expect(parser* parser, token_type type){
    token* result = parser_mov(parser);
    if(result->type != type)
        error("[parser]-> line %lu: expected a token of type '%d', received a token of type '%d'\n", parser->token->line, type, result->type);

    return result;
}

static unsigned is_keyword(token* token, const char* value){
    if(token->type != TOKEN_IDENT)
        return 1;

    return strcmp(token->value, value) == 0 ? 0 : 1;
}

static ast_node* parse_stmt(parser* parser);

static ast_node* parse_expr(parser* parser);

static ast_node* parse_type(parser* parser){
    parser->state = PARSER_FWD;

    if(parser->token->type == TOKEN_IDENT){
        if(strcmp(parser->token->value, "int") != 0)
            error("[parser]-> line %lu: only the \"int\" type is supported\n", parser->token->line);

        ast_node* ident_t = init_ast_node(AST_IDENT_T, parser->token->line);
        ident_t->ident.value = parser_mov(parser)->value;

        return ident_t;
    }

    error("[parser]-> line %lu: literal types are not yet supported\n", parser->token->type);

    ast_node* lit_t = init_ast_node(AST_LIT_T, parser->token->line);
    lit_t->lit_t.lit = parse_expr(parser);

    return lit_t;
}

static ast_node* parse_primary(parser* parser){
    parser->state = PARSER_FWD;

    switch(parser->token->type){
        case TOKEN_LPAREN: {
            parser_mov(parser);
            ast_node* expr = parse_expr(parser);
            parser_expect(parser, TOKEN_RPAREN);

            return expr;
        }
        case TOKEN_INT: {
            ast_node* integer = init_ast_node(AST_INT, parser->token->line);
            integer->integer.value = atoi(parser_mov(parser)->value);

            return integer;
        }
        case TOKEN_IDENT: {
            ast_node* ident = init_ast_node(AST_IDENT, parser->token->line);
            ident->ident.value = parser_mov(parser)->value;

            return ident;
        }
        case TOKEN_STR: {
            ast_node* string = init_ast_node(AST_STR, parser->token->line);
            string->str.value = parser->token->value;

            return string;
        }
        default: break;
    }

    error("[parser]-> line %lu: expected expression\n", parser->token->line);
    return 0;
}

static ast_node* parse_mul(parser* parser){
    parser->state = PARSER_FWD;

    ast_node* left = parse_primary(parser);
    
    while(parser->token->type == TOKEN_STAR || parser->token->type == TOKEN_SLASH){
        ast_node* bin = init_ast_node(AST_BIN, parser->token->line);
        bin->bin.left = left;
        bin->bin.op = parser_mov(parser)->value;
        bin->bin.right = parse_primary(parser);

        left = bin;
    }

    return left;
}

static ast_node* parse_add(parser* parser){
    parser->state = PARSER_FWD;

    ast_node* left = parse_mul(parser);
    
    while(parser->token->type == TOKEN_PLUS || parser->token->type == TOKEN_MINUS){
        ast_node* bin = init_ast_node(AST_BIN, parser->token->line);
        bin->bin.left = left;
        bin->bin.op = parser_mov(parser)->value;
        bin->bin.right = parse_mul(parser);

        left = bin;
    }

    return left;
}

static ast_node* parse_assign(parser* parser){
    parser->state = PARSER_FWD;

    unsigned long left_ln = parser->token->line;
    ast_node* left = parse_add(parser);

    if(parser->token->type == TOKEN_EQ){
        if(left->type != AST_IDENT)
            error("[parser]-> line %lu: expected left of assignment binary to be of type '%d', instead got type '%d'\n", left_ln, AST_IDENT, left->type);
    
        ast_node* bin = init_ast_node(AST_BIN, left_ln);
        bin->bin.left = left;
        bin->bin.op = parser_mov(parser)->value;
        bin->bin.right = parse_add(parser);

        left = bin;
    }

    return left;
}

static ast_node* parse_expr(parser* parser){
    return parse_assign(parser);
}

static ast_node* parse_func(parser* parser){
    parser->state = PARSER_FWD;
    unsigned long ln = parser->token->line;
    parser_expect(parser, TOKEN_IDENT);
    
    ast_node* def = init_ast_node(AST_FUNC_DEF, ln);
    def->func.name = parser_expect(parser, TOKEN_IDENT)->value;
    def->func.params = init_linked_list();
    def->func.body = init_linked_list();

    if(parser->token->type == TOKEN_LPAREN){
        parser_mov(parser);

        while(parser->token->type != TOKEN_RPAREN){
            linked_list_append(def->func.params, parser_expect(parser, TOKEN_IDENT)->value);
            parser_mov(parser);
            linked_list_append(def->func.params, mk_char_ptr("!type_barrier"));
            linked_list_append(def->func.params, parse_type(parser));

            if(parser->token->type == TOKEN_RPAREN)
                break;
            parser_expect(parser, TOKEN_COMMA);
        }

        parser_expect(parser, TOKEN_RPAREN);
    }

    if(parser->token->type == TOKEN_RARROW){
        parser_mov(parser);
        def->func.ret_type = parse_type(parser);
    }else{
        def->func.ret_type = init_ast_node(AST_IDENT_T, parser->token->line);
        def->func.ret_type->ident.value = mk_char_ptr("int");
    }

    if(parser->token->type != TOKEN_LBRACE)
        linked_list_append(def->func.body, parse_stmt(parser));
    else{
        parser_mov(parser);
        while(parser->token->type != TOKEN_RBRACE && parser->token->type != TOKEN_EOF)
            linked_list_append(def->func.body, parse_stmt(parser));
        parser_expect(parser, TOKEN_RBRACE);
    }

    if(parser->token->type == TOKEN_SEMI)
        parser_mov(parser);

    return def;
}

static ast_node* parse_ret(parser* parser){
    parser->state = PARSER_FWD;
    unsigned long ln = parser->token->line;
    parser_expect(parser, TOKEN_IDENT);

    ast_node* ret = init_ast_node(AST_RET, ln);
    ret->ret.value = parse_expr(parser);

    parser_expect(parser, TOKEN_SEMI);
    return ret;
}

static ast_node* parse_var_def(parser* parser){
    parser->state = PARSER_FWD;
    unsigned long ln = parser->token->line;
    parser_expect(parser, TOKEN_IDENT);

    ast_node* var = init_ast_node(AST_VAR_DEF, ln);
    var->var.name =  parser_expect(parser, TOKEN_IDENT)->value;
    parser_expect(parser, TOKEN_COLON);
    parse_type(parser);
    parser_expect(parser, TOKEN_EQ);
    var->var.value = parse_expr(parser);

    parser_expect(parser, TOKEN_SEMI);
    return var;
}

static ast_node* parse_scope(parser* parser){
    parser->state = PARSER_FWD;
    unsigned long ln = parser->token->line;
    parser_expect(parser, TOKEN_LBRACE);

    ast_node* scope = init_ast_node(AST_SCOPE, ln);
    scope->scope.body = init_linked_list();

    while(parser->token->type != TOKEN_EOF && parser->token->type != TOKEN_RBRACE)
        linked_list_append(scope->scope.body, parse_stmt(parser));

    parser_expect(parser, TOKEN_RBRACE);

    if(parser->token->type == TOKEN_SEMI)
        parser_mov(parser);
    return scope;
}

static ast_node* parse_if(parser* parser, unsigned kwrd){
    parser->state = PARSER_FWD;
    unsigned long ln = parser->token->line;
    if(kwrd == 1)
        parser_expect(parser, TOKEN_IDENT);
    parser_expect(parser, TOKEN_LPAREN);

    ast_node* tree = init_ast_node(AST_IF_TREE, ln);
    tree->if_tree.condition = parse_expr(parser);
    tree->if_tree.else_body = 0;
    parser_expect(parser, TOKEN_RPAREN);
    tree->if_tree.then_scope = parse_scope(parser);

    if(is_keyword(parser->token, "else") == 0){
        parser_mov(parser);
        tree->if_tree.else_body = parse_scope(parser);
    }
    if(is_keyword(parser->token, "elif") == 0){
        parser_mov(parser);
        tree->if_tree.else_body = parse_if(parser, 0);
    }
    
    return tree;
}

static ast_node* parse_stmt(parser* parser){
    if(is_keyword(parser->token, "fn") == 0)
        return parse_func(parser);
    if(is_keyword(parser->token, "ret") == 0)
        return parse_ret(parser);
    if(is_keyword(parser->token, "vr") == 0)
        return parse_var_def(parser);
    if(is_keyword(parser->token, "if") == 0)
        return parse_if(parser, 1);
    if(parser->token->type == TOKEN_LBRACE)
        return parse_scope(parser);

    ast_node* expr = parse_expr(parser);
    parser_expect(parser, TOKEN_SEMI);
    return expr;
}

linked_list* parser_parse(parser* parser){
    linked_list* nodes = init_linked_list();

    while(parser->pos < parser->tokens->size - 2)
        linked_list_append(nodes, parse_stmt(parser));

    return nodes;
}