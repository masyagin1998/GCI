#include "lexer.h"
#include "parser.h"
#include "bytecode-generator.h"
#include "virtual-machine.h"

#include <stdio.h>

#define MAX_FNAME_SIZE 1024

#define STACKSIZE 1024
#define HEAPSIZE  35

void run_single_test(unsigned num, const char*fname, int exp)
{
    int r;
    
    lexer_type_t  lexer;
    parser_type_t parser;
    bytecode_generator_type_t bc_gen;
    virtual_machine_type_t vm;

    struct UNIT_AST*unit;
    bytecode_type_t bc;
    
    int got;

    lexer = create_lexer();
    lexer_conf_from_file(lexer, fname);

    parser = create_parser();
    parser_conf(parser, lexer);
    r = parser_parse(parser, &unit);
    if (r != PARSER_OK) {
        printf("%u) PARSER ERROR\n", num);
        exit(0);
    }

    lexer_free(lexer);
    parser_free(parser);

    bc_gen = create_bytecode_generator();
    bytecode_generator_conf(bc_gen, unit);

    r = bytecode_generator_generate(bc_gen, &bc);
    if (r != BYTECODE_GENERATOR_OK) {
        printf("%u) BYTECODE GENERATOR ERROR\n", num);
        exit(0);
    }

    vm = create_virtual_machine();
    virtual_machine_conf(vm, bc, STACKSIZE, HEAPSIZE);
    got = virtual_machine_run(vm);
    bytecode_free(bc);
    virtual_machine_free(vm);
    
    printf("%u) EXP = %d; GOT = %d; %s\n", num, exp, got, exp == got ? "PASSED" : "FAILED");
    if (exp != got) {
        exit(0);
    }
}

#define SYNTAX_TESTS_NUM 10

static const char syntax_tests_fnames[SYNTAX_TESTS_NUM][MAX_FNAME_SIZE] = {
    "data/tests/syntax/01.js",
    "data/tests/syntax/02.js",
    "data/tests/syntax/03.js",
    "data/tests/syntax/04.js",
    "data/tests/syntax/05.js",
    "data/tests/syntax/06.js",
    "data/tests/syntax/07.js",
    "data/tests/syntax/08.js",
    "data/tests/syntax/09.js",
    "data/tests/syntax/10.js",
};

static const int syntax_tests_results[SYNTAX_TESTS_NUM] = {
    55,
    72,
    0,
    3628800,
    45,
    1,
    2,
    25,
    15,
    -100,
};

void run_syntax_tests()
{
    unsigned i;
    
    printf("RUNNING SYNTAX TESTS:\n");
    for (i = 0; i < SYNTAX_TESTS_NUM; i++) {
        run_single_test(i + 1, syntax_tests_fnames[i], syntax_tests_results[i]);
    }
    printf("ALL SYNTAX TESTS PASSED!\n");
}

#define GC_TESTS_NUM 10

static const char gc_tests_fnames[GC_TESTS_NUM][MAX_FNAME_SIZE] = {
    "data/tests/gc/01.js",
    "data/tests/gc/02.js",
    "data/tests/gc/03.js",
    "data/tests/gc/04.js",
    "data/tests/gc/05.js",
    "data/tests/gc/06.js",
    "data/tests/gc/07.js",
    "data/tests/gc/08.js",
    "data/tests/gc/09.js",
    "data/tests/gc/10.js",                                                                     
};

static const int gc_tests_results[SYNTAX_TESTS_NUM] = {
    8,
    45,
    5,
    220,
    45,
    1,
    2,
    25,
    15,
    -100,
};

void convention() { FILE*f = file_open("conv", "w"); fclose(f); }

void run_gc_tests()
{
    unsigned i;
    
    printf("RUNNING GC TESTS:\n");
    for (i = 0; i < GC_TESTS_NUM; i++) {
        run_single_test(i + 1, gc_tests_fnames[i], gc_tests_results[i]);
    }
    printf("ALL GC TESTS PASSED!\n");
}

int main(int argc, char**argv)
{
    PREFIX_UNUSED(argc);
    PREFIX_UNUSED(argv);

    printf("RUNNING TESTS:\n\n");
    run_syntax_tests();
    run_gc_tests();
    printf("ALL TESTS PASSED:\n\n");

    return 0;
}
