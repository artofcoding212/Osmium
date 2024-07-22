#include "include/codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
    if(argc < 2)
        error("[main]-> no file(s) provided\n");

    char* file_name = mk_char_ptr("a");
    if(argc >= 4 && strcmp(argv[2], "-out") == 0)
        file_name = argv[3];

    char* s_name = calloc(strlen(file_name) + 3, sizeof(char));
    strcat(s_name, file_name);
    strcat(s_name, ".s");
    char* o_name = calloc(strlen(file_name) + 3, sizeof(char));
    strcat(o_name, file_name);
    strcat(o_name, ".o");
    char* out_name = calloc(strlen(file_name) + 3, sizeof(char));
    strcat(out_name, file_name);
    strcat(out_name, ".out");

    char* assemble_fmt = "nasm %s -o %s -felf64";
    char* assemble_cmd = calloc(strlen(file_name) + ((strlen(file_name) + 4) * 2), sizeof(char));
    sprintf(assemble_cmd, assemble_fmt, s_name, o_name);
    char* link_fmt = "ld %s -o %s";
    char* link_cmd = calloc(strlen(file_name) + ((strlen(file_name) + 4) * 2), sizeof(char));
    sprintf(link_cmd, link_fmt, o_name, out_name);

    linked_list* tokens = tokenize(read_file(argv[1]));
    linked_list* tree = parser_parse(init_parser(tokens));
    char* s = codegen_gen(init_codegen(tree));
    write_file(s_name, s);
    system(assemble_cmd);
    system(link_cmd);

    free(assemble_cmd);
    free(link_cmd);
    free(out_name);
    free(tokens);
    free(s_name);
    free(o_name);
    free(tree);
    free(s);
    return 0;
}