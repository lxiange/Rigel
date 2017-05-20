
#include "common.h"
#include "gen_cmm.h"
#include "emi.h"


// Why use global var? Because I don't care.

#define TESTS_PATH "generated_cmm/"

int main(int argc, char **argv) {
    mkdir_ifnot_exist("tmp_dir");
    debug_print("main begin");
    const char *cc_path = "./Cmm-Compiler-master/Code/parser";
//    test_cmm_file("test_case/bubble-sort.c");
//    test_cmm_file("test_case/ttttt.c");
    CUR_SEED = (unsigned) time(NULL);
    srand(CUR_SEED);
    generate_tests(TESTS_PATH, 10);
    test_compiler(cc_path, TESTS_PATH);
    fprintf(stderr,
            ANSI_COLOR_GREEN "Cong! No bug found!\n" ANSI_COLOR_CYAN "You must be a lucky guy.\n" ANSI_COLOR_RESET);
    return 0;
}
