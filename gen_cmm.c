//
// Created by Lxiange on 2017/4/22.
//
#define _BSD_SOURCE

#include "common.h"
#include "gen_cmm.h"



void generate_cmm(const char *outfile) {
    const char *cmd = "bash -c "
            "\"./csmith/src/csmith --probability-configuration csmith/prob.txt "
            "--no-argc --no-bitfields --no-checksum --no-compound-assignment --concise --no-consts "
            "--no-pre-incr-operator --no-pre-decr-operator --no-post-incr-operator --no-post-decr-operator "
            "--no-unary-plus-operator --no-jumps --no-longlong --no-int8 --no-uint8 --no-float --no-math64 "
            "--max-array-dim 1 --max-block-depth 1 --max-expr-complexity 1 --max-pointer-depth 1 --quiet "
            "--no-unions --no-volatiles --no-volatile-pointers --no-const-pointers --no-global-variables "
            "--no-builtins --no-inline-function --ccomp --no-return-structs --no-arg-structs --no-vol-struct-union-fields "
            "--no-const-struct-union-fields --no-dangling-global-pointers --no-return-dead-pointer "
            "--strict-float --no-signed-char-index --no-safe-math --no-force-non-uniform-arrays "
            "--fresh-array-ctrl-var-names --arrays \" \n";
    FILE *p_file = popen(cmd, "r");
    fprintf(stderr, "%s", cmd);
    FILE *cmm_code = fopen(outfile, "w");
    char buf[BUFSIZ];
    while (fgets(buf, BUFSIZ, p_file) != NULL) {
        fputs(buf, cmm_code);
    }
    pclose(p_file);
    fclose(cmm_code);
}

void mkdir_not_exist(const char *dirname) {
    struct stat st = {0};

    if (stat(dirname, &st) == -1) {
#ifdef _WIN32
        mkdir(dirname);
#else
        mkdir(dirname, 0700);
#endif

    }
}

void generate_tests(const char *dirname, int tests_num) {
    size_t dirname_len = strlen(dirname);
    assert(dirname[dirname_len - 1] == '/');
    mkdir_not_exist(dirname);
    char filename_buf[PATH_MAX];
    for (int i = 0; i < tests_num; i++) {
        sprintf(filename_buf, "%s%d.c", dirname, i);
        generate_cmm(filename_buf);
    }
}