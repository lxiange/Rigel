//
// Created by Lxiange on 2017/4/22.
//

#ifndef TEST_CLANG_AST_GEN_CMM_H
#define TEST_CLANG_AST_GEN_CMM_H

void generate_cmm(const char *outfile, unsigned seed);

void generate_tests(const char *dst_dir, int cmm_file_num);

void mkdir_ifnot_exist(const char *dir_name);

#endif //TEST_CLANG_AST_GEN_CMM_H
