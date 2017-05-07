//
// Created by Lxiange on 2017/4/24.
//

#ifndef TEST_CLANG_AST_EMI_H
#define TEST_CLANG_AST_EMI_H

#include "common.h"

enum BugType {
    COMPILE_TIME_OUT = 1,
    CODE_DEAD_LOOP,
    NO_IR_CODE_FOUND,
    OUTPUT_MISMATCH,
    FALSE_ACCEPT,
    TRUE_REJECT,
};

extern unsigned CUR_SEED;

extern char inserted_pos[MAX_LINE_NUM];
extern char refactor_pos[MAX_LINE_NUM];
extern char coverage_map[MAX_LINE_NUM];

int rand_100();

void test_compiler(const char *cc_path, const char *test_case_dir);

void report_bug(enum BugType e);

#define REPORT_BUG(e) do { debug_print("found bug, BugType:%d", e);report_bug(e);} while (0)


#endif //TEST_CLANG_AST_EMI_H
