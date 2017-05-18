//
// Created by Lxiange on 2017/4/24.
//

#include "emi.h"
#include "clang_helper.h"


const char *STMT_TABLE[] = {
        "int s%d = 1;\ns%d = s%d + 1;\n",
        "\n",
//        "233;\n",
        "int w%d = 42343;\nw%d = 1 * w%d;\n",
};

const char *CMM_FLAG_TABLE[] = {
#ifdef ENABLE_CMM_FLAG
"-O0",
"-O1",
"-O2",
"-O3",
#endif
        NULL,
};

const char *BUG_TYPE_TABLE[] = {
        "not a bug.",
        "COMPILE_TIME_OUT",
        "CODE_DEAD_LOOP",
        "NO_IR_CODE_FOUND",
        "OUTPUT_MISMATCH",
        "FALSE_ACCEPT",
        "TRUE_REJECT",
};

char inserted_pos[MAX_LINE_NUM];
char refactor_pos[MAX_LINE_NUM];
char coverage_map[MAX_LINE_NUM];
unsigned CUR_SEED;

void report_bug(enum BugType e) {
    fprintf(stderr, "bug detail: %s\n", BUG_TYPE_TABLE[e]);
    fprintf(stderr, "current seed: %d\n", CUR_SEED);
    assert(0);
}


int rand_100() {
    return rand() * 100 / RAND_MAX;
}


static void exec_ir_driver(const char *inserted_ir, char *output_buf, int enable_cov, const char *input_file) {
    char cmd_buf[PATH_MAX];
    char buf[MAX_COL_NUM];
    int flag = 0;
    FILE *ir_sim_out;
#ifdef ENABLE_INPUT
    //    input_file = "test_case/test_input.c.input";
    if (fopen(input_file, "r") == NULL) {
        sprintf(cmd_buf, "timeout 5 python3 irsim-cli.py %s", inserted_ir);
    } else {
        sprintf(cmd_buf, "cat %s | timeout 5 python3 irsim-cli.py %s", input_file, inserted_ir);
    }
#else
    sprintf(cmd_buf, "timeout 5 python3 irsim-cli.py %s", inserted_ir);
#endif
    debug_print("exec command: %s", cmd_buf);
    ir_sim_out = popen(cmd_buf, "r");
    while (fgets(buf, MAX_COL_NUM, ir_sim_out) != NULL) {
        debug_print("ir output buf: %s", buf);
        if (memcmp(buf, "stdout:  ", 9) != 0) {
            continue;   // we just care stdout.
        }

        if (!strcmp(buf, "stdout:  " BEGIN_MAGIC "\n")) {
            flag = 1;
        } else if (!strcmp(buf, "stdout:  " END_MAGIC "\n")) {
            flag = 0;
        } else if (!flag) {
            strcat(output_buf, buf + 9);// TODO: replace strcat to improve performance.
        } else if (enable_cov) {
            int line_num = atoi(buf + 9);
            coverage_map[line_num] = 1;
            coverage_map[line_num - 1] = 1;// TODO: NOT accurate.
        } else {
            //do nothing;
        }
    }
    int return_code = WEXITSTATUS(pclose(ir_sim_out));

    if (return_code == 124) {
        debug_print("return code:%d", return_code);
        REPORT_BUG(CODE_DEAD_LOOP);
    }
    if (enable_cov) {
        for (int i = 0; i < MAX_LINE_NUM; i++) {
            if (coverage_map[i] != 1 && inserted_pos[i] == 1) {
                debug_print("refactor_pos: %d", i);
                refactor_pos[i] = 1;
            }
        }
    }
}


static void exec_ir_for_cov(const char *inserted_ir, char *output_buf, const char *input_file) {
    exec_ir_driver(inserted_ir, output_buf, 1, input_file);
}

static void exec_ir_code(const char *inserted_ir, char *output_buf, const char *input_file) {
    exec_ir_driver(inserted_ir, output_buf, 0, input_file);
}

/*
 * return 0 if no error
 */
static int cmm_compile(const char *cc_path, const char *cmm_flag, const char *test_file) {
    char cmd_buf[PATH_MAX];
    if (cmm_flag != NULL) {
        sprintf(cmd_buf, "timeout 5 %s %s %s", cc_path, cmm_flag, test_file);
    } else {
        sprintf(cmd_buf, "timeout 5 %s %s", cc_path, test_file);
    }
    debug_print("exec command: %s\n", cmd_buf);
    int ret = system(cmd_buf);
    if (ret != 0) {
        debug_print("cmm compiler exit unnormally.");
    }
    if (access(IR_FILE_NAME, F_OK) != -1) {
        // file exists
        debug_print("found ir code");
    } else {
        // file doesn't exist
        REPORT_BUG(NO_IR_CODE_FOUND);
    }
    debug_print("compiled %s", test_file);
    if (ret == 124) {
        fprintf(stderr, "Compile %s timeout.", test_file);
        REPORT_BUG(COMPILE_TIME_OUT);
    }
    return ret == 0 ? 0 : 1;
}

static int sem_check(const char *cc_path, const char *cmm_flag, const char *test_file) {
    int is_illegal = sem_check_by_clang(test_file);

    if (is_illegal != cmm_compile(cc_path, cmm_flag, test_file)) {
        if (is_illegal) {
            debug_print("false accepted.");
            REPORT_BUG(FALSE_ACCEPT);
        } else {
            debug_print("true accepted.");
            REPORT_BUG(TRUE_REJECT);
        }
    } else {
        debug_print("syntax check pass, is_illegal: %d\n", is_illegal);
    }
    return is_illegal;
}

static void random_test(const char *cc_path, const char *cmm_flag, const char *test_file,
                        const char *origin_output, const char *input_file) {


    debug_print("test cmm file:%s", test_file);
    FILE *f_in = fopen(test_file, "r");
    FILE *f_out = fopen(RANDOM_GEN_FILE, "w+");
    char buf[MAX_LINE_NUM];
    for (int cur_line = 1; fgets(buf, MAX_COL_NUM, f_in) != NULL; cur_line++) {
        //todo: do it better.
        if (inserted_pos[cur_line] && rand_100() < 10) {
            int choice = rand() * (sizeof(STMT_TABLE) / sizeof(char *)) / RAND_MAX;
            debug_print("choice: %d:\n%s", choice, STMT_TABLE[choice]);
            int rand_num = rand();
            fprintf(f_out, STMT_TABLE[choice], rand_num, rand_num, rand_num);
        }

        if (refactor_pos[cur_line] && rand_100() < 80) {// p=0.8 to change code.
            int choice = rand() * (sizeof(STMT_TABLE) / sizeof(char *)) / RAND_MAX;
            debug_print("choice: %d:\n%s", choice, STMT_TABLE[choice]);
            int rand_num = rand();
            fprintf(f_out, STMT_TABLE[choice], rand_num, rand_num, rand_num);
        } else {
            fputs(buf, f_out);
        }
    }
    fclose(f_in);
    fclose(f_out);

    if (sem_check(cc_path, cmm_flag, RANDOM_GEN_FILE) != 0) {
        debug_print("random_gen_file is not a valid c file.");
        remove(RANDOM_GEN_FILE);
        remove(IR_FILE_NAME);
        return;
    }

    char *output_buf = (char *) calloc(STDOUT_BUF_SIZ, sizeof(char));
    exec_ir_code(IR_FILE_NAME, output_buf, input_file);
    if (strcmp(output_buf, origin_output) != 0) {
        REPORT_BUG(OUTPUT_MISMATCH);
    }
    free(output_buf);
    remove(RANDOM_GEN_FILE);
    remove(IR_FILE_NAME);
}


static void insert_watch_point(const char *in_file, const char *out_file) {
    FILE *f_in = fopen(in_file, "r");
    FILE *f_out = fopen(out_file, "w");
    fprintf(f_out, "int my_func(int line_num){\n"
            "write(" BEGIN_MAGIC ");\n"
            "write(line_num);\n"
            "write(" END_MAGIC ");\n"
            "return 0;\n"
            "}\n");
    char buf[MAX_COL_NUM];
    for (int cur_line = 1; fgets(buf, MAX_COL_NUM, f_in) != NULL; cur_line++) {
        if (inserted_pos[cur_line]) {
            fprintf(f_out, "my_func(%d);\n", cur_line);
        }
        fputs(buf, f_out);
    }
    fclose(f_in);
    fclose(f_out);
}


static void count_coverage(const char *cc_path, const char *test_file,
                           const char *input_file, char *origin_output) {


    memset(inserted_pos, 0, sizeof(inserted_pos));
    memset(refactor_pos, 0, sizeof(refactor_pos));
    memset(coverage_map, 0, sizeof(coverage_map));

    // generate AST to find where to insert watch point.
    traverse_cmm_ast(test_file);
    insert_watch_point(test_file, INSERTED_C_FILE);
    cmm_compile(cc_path, NULL, INSERTED_C_FILE);
    exec_ir_for_cov(IR_FILE_NAME, origin_output, input_file);
    remove(IR_FILE_NAME);

}


static void test_cmm_file(const char *cc_path, const char *test_file) {
    // Use clang to check semantics.
    if (sem_check(cc_path, NULL, test_file) != 0) {
        debug_print("not a legal c file.");
        return;
    }

    char input_file[1024] = {0};
#ifdef ENABLE_INPUT
    sprintf(input_file, "%s.input", test_file);
#endif

    char *origin_output = (char *) calloc(STDOUT_BUF_SIZ, sizeof(char));

    count_coverage(cc_path, test_file, input_file, origin_output);

    for (int i = 0; i < ITER_TIMES; i++) {
        for (int j = 0; j < sizeof(STMT_TABLE) / sizeof(char *); j++) {
            random_test(cc_path, CMM_FLAG_TABLE[j], test_file, origin_output, input_file);
        }
    }
    free(origin_output);
}

void test_compiler(const char *cc_path, const char *test_case_dir) {
    DIR *FD;
    if (NULL == (FD = opendir(test_case_dir))) {
        fprintf(stderr, "Error : Failed to open input directory\n");
        return;
    }
    struct dirent *in_file;
    char path_buf[PATH_MAX];
    while ((in_file = readdir(FD))) {
        if (!memcmp(in_file->d_name, ".", 1)) {
            continue;
        }
        sprintf(path_buf, "%s%s", test_case_dir, in_file->d_name);
        debug_print("meet file: %s\n", path_buf);
        test_cmm_file(cc_path, path_buf);
        remove(INSERTED_C_FILE);
        remove(IR_FILE_NAME);
        remove(RW_DECLARED_SOURCE);
    }
}
