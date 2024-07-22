#include "include/codegen.h"
#include <stdlib.h>
#include <string.h>

typedef struct int_cntr {
    int val;
} int_cntr;

int_cntr* init_int_cntr(int value){
    int_cntr* cntr = calloc(1, sizeof(struct int_cntr));
    cntr->val = value;

    return cntr;
}

rt_var* init_rt_var(char* name, unsigned long loc){
    rt_var* var = calloc(1, sizeof(struct rt_var));
    var->name = name;
    var->loc = loc;

    return var;
}

rt_scope* init_rt_scope(rt_scope* parent, unsigned short is_main){
    rt_scope* scope = calloc(1, sizeof(struct rt_scope));
    scope->parent = parent;
    scope->is_main = is_main;
    scope->vars = init_linked_list();

    return scope;
}

rt_var* rt_scope_lookup(rt_scope* scope, char* name){
    for(linked_list_node* node = scope->vars->head; node != 0; node = node->next)
        if(strcmp(((rt_var*)node->value)->name, name) == 0)
            return node->value;

    if(scope->parent)
        return rt_scope_lookup(scope->parent, name);
    
    return 0;
}

static void codegen_push(codegen* codegen, char* reg){
    char* str = str_fmt("push %s\n", strlen(reg), reg);
    str_append(&codegen->result, str, 0);

    codegen->stack_size++;
    free(str);
}

static void codegen_pop(codegen* codegen, char* reg){
    char* str = str_fmt("pop %s\n", strlen(reg), reg);
    str_append(&codegen->result, str, 0);

    codegen->stack_size--;
    free(str);
}

static void codegen_gen_node(codegen* codegen, rt_scope* scope, ast_node* node);

static void codegen_gen_if_pred(codegen* codegen, rt_scope* scope, ast_node* node, unsigned long end_label){
    switch(node->type){
        case AST_SCOPE: {
            codegen_gen_node(codegen, scope, node);

            char* jump_fmt = str_fmt("jmp __CORE_LABEL_%d\n", 12, end_label);
            str_append(&codegen->result, jump_fmt, 0);
            free(jump_fmt);

            break;
        }
        case AST_IF_TREE: {
            codegen_gen_node(codegen, scope, node->if_tree.condition);
            codegen_pop(codegen, "rax");
            str_append(&codegen->result, "test rax, rax\n", 0);

            unsigned long else_label = codegen->label_count++;

            char* else_fmt = str_fmt("jz __CORE_LABEL_%d\n", 12, else_label);
            str_append(&codegen->result, else_fmt, 0);
            free(else_fmt);

            codegen_gen_node(codegen, scope, node->if_tree.then_scope);

            char* jmp_fmt = str_fmt("jmp __CORE_LABEL_%d\n", 12, end_label);
            str_append(&codegen->result, jmp_fmt, 0);

            char* else_label_fmt = str_fmt("__CORE_LABEL_%d:\n", 12, else_label);
            str_append(&codegen->result, else_label_fmt, 0);
            free(else_label_fmt);
            if(node->if_tree.else_body == 0)
                str_append(&codegen->result, jmp_fmt, 0);
            else
                codegen_gen_if_pred(codegen, scope, node->if_tree.else_body, end_label);
            free(jmp_fmt);

            char* end_label_fmt = str_fmt("__CORE_LABEL_%d:\n", 12, end_label);
            str_append(&codegen->result, end_label_fmt, 0);
            free(end_label_fmt);

            break;
        }
        default: error("[codegen]-> line %lu: ast node of type '%d' is not recognized as a valid if statement predicate\n", node->line, node->type);
    }
}

static void codegen_gen_node(codegen* codegen, rt_scope* scope, ast_node* node){
    switch(node->type){
        case AST_VAR_DEF: {
            if(rt_scope_lookup(scope, node->var.name) != 0)
                error("[codegen]-> line %lu: variable '%s' is already defined\n", node->line, node->var.name);

            linked_list_append(scope->vars, init_rt_var(node->var.name, codegen->stack_size));

            if(node->var.value == 0){
                ast_node* int_node = init_ast_node(AST_INT, node->line);
                int_node->integer.value = 0;

                node->var.value = int_node;
            }

            codegen_gen_node(codegen, scope, node->var.value);
            break;
        }
        case AST_FUNC_DEF: {
            if(strcmp(node->func.name, "main") != 0)
                error("[codegen]-> line %lu: non-main function definitions are not yet supported\n", node->line);

            rt_scope* new_scope = init_rt_scope(scope, 1);
            str_append(
                &codegen->result,
                "global _start\n"
                "_start:\n",
                0
            );
            
            for(linked_list_node* body_node = node->func.body->head; body_node != 0; body_node = body_node->next)
                codegen_gen_node(codegen, new_scope, body_node->value);

            break;
        }
        case AST_RET: {
            if(scope->is_main != 1)
                error("[codegen]-> line %lu: non-exit returns are not yet supported\n", node->line);

            codegen_gen_node(codegen, scope, node->ret.value);
            str_append(&codegen->result, "mov rax, 60\n", 0);
            codegen_pop(codegen, "rdi");
            str_append(&codegen->result, "syscall\n", 0);

            break;
        }
        case AST_SCOPE: {
            rt_scope* new_scope = init_rt_scope(scope, scope->is_main);
            linked_list_append(codegen->scopes, init_int_cntr(scope->vars->size));

            for(linked_list_node* body_node = node->scope.body->head; body_node != 0; body_node = body_node->next)
                codegen_gen_node(codegen, new_scope, body_node->value);

            int lookup_spot = scope->vars->size - 1;
            linked_list_node* lookup_node = linked_list_get(scope->vars, lookup_spot < 0 ? 0 : lookup_spot);
            unsigned pop_count = (new_scope->vars->size + 1) - (lookup_node == 0 ? 0 : ((int_cntr*)lookup_node->value)->val);
            char* add_str = str_fmt("add rsp, %d\n", 12, pop_count * 8);
            str_append(&codegen->result, add_str, 0);
            free(add_str);

            codegen->stack_size -= pop_count;

            for(unsigned index = 0; index < pop_count; index++)
                linked_list_destroy(scope->vars, scope->vars->size);
            linked_list_destroy(codegen->scopes, codegen->scopes->size);
            break;
        }
        case AST_IF_TREE: {
            codegen_gen_node(codegen, scope, node->if_tree.condition);
            codegen_pop(codegen, "rax");
            str_append(&codegen->result, "test rax, rax\n", 0);

            unsigned long end_label = codegen->label_count++;
            unsigned long else_label = codegen->label_count++;

            char* else_fmt = str_fmt("jz __CORE_LABEL_%d\n", 12, else_label);
            str_append(&codegen->result, else_fmt, 0);
            free(else_fmt);

            codegen_gen_node(codegen, scope, node->if_tree.then_scope);

            char* jmp_fmt = str_fmt("jmp __CORE_LABEL_%d\n", 12, end_label);
            str_append(&codegen->result, jmp_fmt, 0);

            char* else_label_fmt = str_fmt("__CORE_LABEL_%d:\n", 12, else_label);
            str_append(&codegen->result, else_label_fmt, 0);
            free(else_label_fmt);
            if(node->if_tree.else_body == 0)
                str_append(&codegen->result, jmp_fmt, 0);
            else
                codegen_gen_if_pred(codegen, scope, node->if_tree.else_body, end_label);
            free(jmp_fmt);

            char* end_label_fmt = str_fmt("__CORE_LABEL_%d:\n", 12, end_label);
            str_append(&codegen->result, end_label_fmt, 0);
            free(end_label_fmt);

            break;
        }
        case AST_BIN: {
            if(strcmp(node->bin.op, "=") == 0){
                rt_var* var = rt_scope_lookup(scope, node->bin.left->ident.value);
                if(var == 0)
                    error("[codegen]-> line %lu: cannot assign to undeclared variable '%s'\n", node->line, node->bin.left->ident.value);

                codegen_gen_node(codegen, scope, node->bin.right);
                codegen_pop(codegen, "rax");

                char* mov_fmt = str_fmt("mov [rsp + %d], rax\n", 12, (codegen->stack_size - var->loc - 1) * 8);
                str_append(&codegen->result, mov_fmt, 2);
                free(mov_fmt);

                break;
            }

            codegen_gen_node(codegen, scope, node->bin.right);
            codegen_gen_node(codegen, scope, node->bin.left);
            codegen_pop(codegen, "rax");
            codegen_pop(codegen, "rbx");

            if(strcmp(node->bin.op, "+") == 0)
                str_append(&codegen->result, "add rax, rbx\n", 0);
            if(strcmp(node->bin.op, "-") == 0)
                str_append(&codegen->result, "sub rax, rbx\n", 0);
            if(strcmp(node->bin.op, "-") == 0)
                str_append(&codegen->result, "mul rbx\n", 0);
            if(strcmp(node->bin.op, "/") == 0)
                str_append(&codegen->result, "div rbx\n", 0);

            codegen_push(codegen, "rax");
            break;
        }
        case AST_INT: {
            char* int_fmt = str_fmt("mov rax, %d\n", 12, node->integer.value);
            str_append(&codegen->result, int_fmt, 12);
            free(int_fmt);

            codegen_push(codegen, "rax");
            break;
        }
        case AST_IDENT: {
            rt_var* var = rt_scope_lookup(scope, node->var.name);
            if(var == 0)
                error("[codegen]-> line %lu: couldn't find variable '%s'\n", node->line, node->var.name);
            
            char* ident_fmt = str_fmt("push QWORD [rsp + %d]\n", 12, (codegen->stack_size - var->loc - 1) * 8);
            str_append(&codegen->result, ident_fmt, 2);
            free(ident_fmt);

            codegen->stack_size++;
            break;
        }
        default: error("[codegen]-> line %lu: unrecognized ast node of type '%d'\n", node->line, node->type);
    }
}

codegen* init_codegen(linked_list* tree){
    codegen* codegen = calloc(1, sizeof(struct codegen));
    codegen->tree = tree;
    codegen->stack_size = 0;
    codegen->label_count = 0;
    codegen->scopes = init_linked_list();
    codegen->main_scope = init_rt_scope(0, 0);
    codegen->result = calloc(1, sizeof(char));

    return codegen;
}

char* codegen_gen(codegen* codegen){
    for(linked_list_node* linked_node = codegen->tree->head; linked_node != 0; linked_node = linked_node->next)
        codegen_gen_node(codegen, codegen->main_scope, linked_node->value);

    return codegen->result;
}