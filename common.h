//
// Created by Lxiange on 2017/4/22.
//

#ifndef TEST_CLANG_AST_COMMON_H
#define TEST_CLANG_AST_COMMON_H

#define _BSD_SOURCE


//#define RELEASE_MODE

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>


#define STDOUT_BUF_SIZ 10000
#define MAX_LINE_NUM 10000
#define MAX_COL_NUM 1000
//#define PATH_MAX 1024
#define INSERTED_C_FILE "tmp_dir/inserted_my_func.c"
#define RW_DECLARED_SOURCE "tmp_dir/temp_file_with_rw.c"
#define RANDOM_GEN_FILE "tmp_dir/random_gen_file.c"
#define IR_FILE_NAME "inter.ir"
#define BEGIN_MAGIC "32432423"
#define END_MAGIC "43253623"
// todo: change the name.
#define ITER_TIMES 2

#define ENABLE_LOG 1

#define debug_print(fmt, args...) \
        do { if (ENABLE_LOG) fprintf(stderr, "%s@%d/%s: " fmt "\n", __FILE__, \
                                __LINE__, __func__, ##args); } while (0)


#endif //TEST_CLANG_AST_COMMON_H
