#include "lexer.h"
#include "parser.h"
#include "bytecode-generator.h"
#include "virtual-machine.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

enum INTERPRETER_MODE
{
    INTERPRETER_LEX,
    INTERPRETER_PARSE,
    INTERPRETER_BC,
    INTERPRETER_INTERPRET,
};

#define INTERPRETER_LEX_STR       "lex"
#define INTERPRETER_PARSE_STR     "parse"
#define INTERPRETER_BC_STR        "bc"
#define INTERPRETER_INTERPRET_STR "interpet"

struct INTERPRETER_PARAMS
{
    char in[STR_BUF_SIZE];
    char out[STR_BUF_SIZE];
    enum INTERPRETER_MODE mode;
};

static void print_version(char*interpreter_name)
{
    fprintf(stderr, "%s\n", interpreter_name);
    fprintf(stderr, "GCI V0.1\n");
    fprintf(stderr, "GCI is interpreter of tiny JavaScript-like language with garbage collector and objects.\n");
    fprintf(stderr, "It supports JavaScript-like objects, numbers, if-else-stmts and while loops.\n");
    fprintf(stderr, "Functionality:\n");
    fprintf(stderr, "  - lexer;\n");
    fprintf(stderr, "  - parser\n;");
    exit(0);
}

static void print_help(char*interpreter_name)
{
    fprintf(stderr, "Usage: %s [options] input.js\n", interpreter_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --version (-v)\n");
    fprintf(stderr, "            Print version info.\n");
    fprintf(stderr, "  --help    (-h)\n");
    fprintf(stderr, "            Print this help info.\n");
    fprintf(stderr, "  --in      (-i)\n");
    fprintf(stderr, "            Path to input file (stdin).\n");
    fprintf(stderr, "  --out     (-o)\n");
    fprintf(stderr, "            Path to output file (stdout|stderr) (default: stdout).\n");
    fprintf(stderr, "  --mode    (-m)\n");
    fprintf(stderr, "            Mode (lex|parse|bc|interpret) (default: interpret).\n");
    exit(0);
}

int parse_args(int argc, char**argv, struct INTERPRETER_PARAMS*params)
{
    struct option opts[] = {
        {"version", 0, 0, 'v'},
        {"help",    0, 0, 'h'},
        {"in",      1, 0, 'i'},
        {"out",     1, 0, 'o'},
        {"mode",    1, 0, 'm'},
        {0,0,0,0}
    };

    int c;
    int idx;

    params->in[0] = '\0';
    strncpy(params->out, "stdout", sizeof(params->out));
    params->mode = INTERPRETER_INTERPRET;
    
    while ((c = getopt_long(argc, argv, "i:o:m:vh", opts, &idx)) != -1) {
        switch (c) {
        case 'v':
            print_version(argv[0]);
            break;
        case 'h':
            print_help(argv[0]);
            break;
        case 'i': {
            strncpy(params->in, optarg, sizeof(params->in));
            break;   
        }
        case 'o':
            strncpy(params->out, optarg, sizeof(params->out));
            break;
        case 'm':
            if (strcmp(optarg, INTERPRETER_LEX_STR) == 0) {
                params->mode = INTERPRETER_LEX;
            } else if (strcmp(optarg, INTERPRETER_PARSE_STR) == 0) {
                params->mode = INTERPRETER_PARSE;
            } else if (strcmp(optarg, INTERPRETER_BC_STR) == 0) {
                params->mode = INTERPRETER_BC;
            } else if (strcmp(optarg, INTERPRETER_INTERPRET_STR) == 0) {
                params->mode = INTERPRETER_INTERPRET;
            } else {
                fprintf(stderr, "Invalid interpreter mode \"%s\"", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            /* do nothing. */
            break;
        }
    }

    return 0;
}

void print_lexer_result(const char*fname, lexer_type_t lexer)
{
    FILE*f = file_open(fname, "w");
    dump_lexer_to_xml_file(f, lexer);
    fclose(f);
}

void print_parser_result(const char*fname, const struct UNIT_AST*unit)
{
    FILE*f = file_open(fname, "w");
    dump_unit_ast_to_xml_file(f, unit);
    fclose(f);
}

void print_bytecode_result(const char*fname, const bytecode_type_t bc)
{
    FILE*f = file_open(fname, "w");
    dump_bytecode_to_xml_file(f, bc);
    fclose(f);
}

#define STACK_SIZE 1024
#define HEAP_SIZE  1024000

int main(int argc, char**argv)
{
    int code;
    
    struct INTERPRETER_PARAMS params;
    lexer_type_t  lexer;
    parser_type_t parser;
    bytecode_generator_type_t bc_gen;
    virtual_machine_type_t vm;

    struct UNIT_AST*unit;
    bytecode_type_t bc;

    parse_args(argc, argv, &params);

    lexer = create_lexer();
    lexer_conf(lexer, params.in);

    if (params.mode == INTERPRETER_LEX) {
        print_lexer_result(params.out, lexer);
        exit(0);
    }

    parser = create_parser();
    parser_conf(parser, lexer);
    code = parser_parse(parser, &unit);

    if (params.mode == INTERPRETER_PARSE) {
        print_parser_result(params.out, unit);
        exit(0);
    }

    bc_gen = create_bytecode_generator();
    bytecode_generator_conf(bc_gen, unit);
    code = bytecode_generator_generate(bc_gen, &bc);

    if (params.mode == INTERPRETER_BC) {
        print_bytecode_result(params.out, bc);
        exit(0);
    }

    vm = create_virtual_machine();
    virtual_machine_conf(vm, bc, STACK_SIZE, HEAP_SIZE);
    virtual_machine_run(vm);

    return 0;
}
