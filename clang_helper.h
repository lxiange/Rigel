//
// Created by Lxiange on 2017/4/24.
//

#ifndef TEST_CLANG_AST_CLANG_HELPER_H
#define TEST_CLANG_AST_CLANG_HELPER_H

#include "common.h"
#include <clang-c/Index.h>

extern char inserted_pos[MAX_LINE_NUM];

void traverse_cmm_ast(const char *test_file);

int sem_check_by_clang(const char *test_file);

#endif //TEST_CLANG_AST_CLANG_HELPER_H
