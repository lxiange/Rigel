//
// Created by Lxiange on 2017/4/22.
//

#include "common.h"
#include "gen_cmm.h"


void generate_cmm(const char *outfile, unsigned seed) {
    const char *cmd_tpl = "bash -c "
            "\"./csmith/src/csmith --probability-configuration csmith/prob.txt --seed %u "
            "--no-argc --no-bitfields --no-checksum --no-compound-assignment --concise --no-consts "
            "--no-pre-incr-operator --no-pre-decr-operator --no-post-incr-operator --no-post-decr-operator "
            "--no-unary-plus-operator --no-jumps --no-longlong --no-int8 --no-uint8 --no-float --no-math64 "
            "--max-array-dim 1 --max-block-depth 1 --max-expr-complexity 1 --max-pointer-depth 1 --quiet "
            "--no-unions --no-volatiles --no-volatile-pointers --no-const-pointers --no-global-variables "
            "--no-builtins --no-inline-function --ccomp --no-return-structs --no-arg-structs --no-vol-struct-union-fields "
            "--no-const-struct-union-fields --no-dangling-global-pointers --no-return-dead-pointer "
            "--strict-float --no-signed-char-index --no-safe-math --no-force-non-uniform-arrays "
            "--fresh-array-ctrl-var-names --arrays \" \n";
    char *cmd = malloc(sizeof(char) * 4096);
    sprintf(cmd, cmd_tpl, seed);
    debug_print("exec command: %s", cmd);
    FILE *p_file = popen(cmd, "r");
    FILE *cmm_code = fopen(outfile, "w");
    char buf[BUFSIZ];
    while (fgets(buf, BUFSIZ, p_file) != NULL) {
        fputs(buf, cmm_code);
    }
    pclose(p_file);
    fclose(cmm_code);
}

void mkdir_ifnot_exist(const char *dir_name) {
    struct stat st = {0};

    if (stat(dir_name, &st) == -1) {
#ifdef _WIN32
        mkdir(dir_name);
#else
        mkdir(dir_name, 0700);
#endif
    }
}

void generate_tests(const char *dst_dir, int cmm_file_num) {
    size_t d_len = strlen(dst_dir);
    assert(dst_dir[d_len - 1] == '/');
    mkdir_ifnot_exist(dst_dir);
    char filename_buf[PATH_MAX];
    for (int i = 0; i < cmm_file_num; i++) {
        sprintf(filename_buf, "%s%d.c", dst_dir, i);
        generate_cmm(filename_buf, (unsigned)rand());
    }
}