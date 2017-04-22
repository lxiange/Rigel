
#define _BSD_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>

#include <clang-c/Index.h>
#include <dirent.h>

#define STDOUT_BUF_SIZ 10000
#define MAX_LINE_NUM 10000
#define MAX_COL_NUM 1000
//#define PATH_MAX 1024
#define INSERTED_C_FILE "inserted_my_func.c"
#define RW_DECLARED_SOURCE "temp_file_with_rw.c"
#define RANDOM_GEN_FILE "random_gen_file.c"
#define BEGIN_MAGIC "32432423"
#define END_MAGIC "43253623"
#define IR_FILE_NAME "inter.ir"
// todo: change the name.
#define ITER_TIMES 2
#define RELEASE_MODE

char inserted_pos[MAX_LINE_NUM];
char refactor_pos[MAX_LINE_NUM];
char coverage_map[MAX_LINE_NUM];
char *CC_PATH;
// Why use global var? Because I don't care.

const char *STMT_TABLE[] = {
        "int s%d = 1;\ns%d = s%d + 1;\n",
        "\n",
        "233;\n",
        "int w%d = 42343;\nw%d = 1 * w%d;\n",
};

void report_bug() {
    fprintf(stderr, "found bug.\n");
    assert(0);
}

bool can_insert(enum CXCursorKind kind) {
    switch (kind) {
        case CXCursor_BinaryOperator:
        case CXCursor_ReturnStmt:
        case CXCursor_IfStmt:
        case CXCursor_WhileStmt:
        case CXCursor_CallExpr:
//        case CXCursor_DeclStmt:// can't delete.
            return true;
        default:
            return false;
    }
}

/*
 * return 0 if no error
 */
int sem_check_by_clang(const char *test_file) {
    FILE *f_in = fopen(test_file, "r");
    FILE *f_out = fopen(RW_DECLARED_SOURCE, "w+");
    fputs("int read();\nvoid write(int x);\n", f_out);
    char buf[MAX_COL_NUM];
    for (int cur_line = 1; fgets(buf, MAX_COL_NUM, f_in) != NULL; cur_line++) {
        fputs(buf, f_out);
    }
    fclose(f_in);
    fclose(f_out);

    CXIndex index = clang_createIndex(0, 1);
    CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(index, RW_DECLARED_SOURCE, 0, NULL, 0, NULL);
    assert(tu);
    unsigned error_num = clang_getNumDiagnostics(tu);

    fprintf(stdout, "%s error_num:%d\n", test_file, error_num);
    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
    return error_num == 0 ? 0 : 1;
}

/*
 * return 0 if no error
 */
int compile_and_check_by_cc(const char *test_file) {
    // TODO: rename it.
    char cmd_buf[PATH_MAX];
    sprintf(cmd_buf, "%s %s", CC_PATH, test_file);
    int ret = system(cmd_buf);
    if (access(IR_FILE_NAME, F_OK) != -1) {
        // file exists
    } else {
        fprintf(stderr, "not exist!!!!!\n");
        report_bug();
        // file doesn't exist
    }
    return ret == 0 ? 0 : 1;
}

void sem_check(const char *test_file) {
    if (sem_check_by_clang(test_file) ^ compile_and_check_by_cc(test_file)) {
        report_bug();
    } else {
        fprintf(stdout, "syntax check pass\n");
    }
}

void insert_watch_point(const char *in_file, const char *out_file) {
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

enum CXChildVisitResult visitor(CXCursor cursor, CXCursor parent /* no use */, CXClientData clientData) {
    CXSourceLocation location = clang_getCursorLocation(cursor);
    if (clang_Location_isFromMainFile(location) == 0)
        return CXChildVisit_Continue;

    enum CXCursorKind cursorKind = clang_getCursorKind(cursor);
    unsigned int line;
    unsigned int column;
    unsigned int offset;
    clang_getSpellingLocation(location, NULL, &line, &column, &offset);
    if (can_insert(cursorKind)) {
        inserted_pos[line] = 1;
    }

    unsigned int curLevel = *((unsigned int *) (clientData));
    unsigned int nextLevel = curLevel + 1;
    CXString kindName = clang_getCursorKindSpelling(cursorKind);
    CXString cursorSpelling = clang_getCursorSpelling(cursor);
//    fprintf(stdout, "%*c%s (%s)\n", curLevel, ' ', (const char *) kindName.data,
//            (const char *) cursorSpelling.data);
    clang_disposeString(kindName);
    clang_disposeString(cursorSpelling);

//    fprintf(stdout, "line:%d,col:%d,off:%d,origin:%d\n", line, column, offset, location.int_data);

    clang_visitChildren(cursor, visitor, &nextLevel);
    return CXChildVisit_Continue;
}

void collect_code_cov_with_input(const char *inserted_ir, char *output_buf, int enable_cov, const char *input_file) {

    char cmd_buf[PATH_MAX];
    char buf[MAX_COL_NUM];
    int flag = 0;
    FILE *ir_sim_out;
    input_file = "test_case/test_input.c.input";
    if (input_file == NULL || fopen(input_file, "r") == NULL) {
        sprintf(cmd_buf, "python3 irsim-cli.py %s", inserted_ir);
    } else {
        sprintf(cmd_buf, "cat %s | python3 irsim-cli.py %s", input_file, inserted_ir);
    }
    ir_sim_out = popen(cmd_buf, "r");
    while (fgets(buf, MAX_COL_NUM, ir_sim_out) != NULL) {
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
    pclose(ir_sim_out);
    if (enable_cov) {
        for (int i = 0; i < MAX_LINE_NUM; i++) {
            if (coverage_map[i] != 1 && inserted_pos[i] == 1) {
                refactor_pos[i] = 1;
            }
        }
    }
}


void visit_file(const char *test_file) {
    CXIndex index = clang_createIndex(1, 1);
    CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(index, test_file, 0, NULL, 0, NULL);
    assert(tu);
    CXCursor rootCursor = clang_getTranslationUnitCursor(tu);

    unsigned int treeLevel = 0;
    clang_visitChildren(rootCursor, visitor, &treeLevel);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
}

int rand_100() {
    return rand() * 100 / RAND_MAX;
}

void random_test(const char *test_file, const char *origin_output, const char *input_file) {
    FILE *f_in = fopen(test_file, "r");
    FILE *f_out = fopen(RANDOM_GEN_FILE, "w+");
    char buf[MAX_LINE_NUM];
    for (int cur_line = 1; fgets(buf, MAX_COL_NUM, f_in) != NULL; cur_line++) {
        if (refactor_pos[cur_line] && rand_100() < 80) {// p=0.8 to change code.
            int choice = rand() * (sizeof(STMT_TABLE) / sizeof(char *)) / RAND_MAX;
            fprintf(stdout, "choice: %d\n", choice);
            int rand_num = rand();
            fprintf(f_out, STMT_TABLE[choice], rand_num, rand_num, rand_num);
//            fputs(STMT_TABLE[choice], f_out);
        } else {
            fputs(buf, f_out);
        }
    }
    fclose(f_in);
    fclose(f_out);
    compile_and_check_by_cc(RANDOM_GEN_FILE);
    char *output_buf = (char *) malloc(sizeof(char) * STDOUT_BUF_SIZ);
    memset(output_buf, 0, sizeof(char) * STDOUT_BUF_SIZ);
    collect_code_cov_with_input(IR_FILE_NAME, output_buf, 0, input_file);
    if (strcmp(output_buf, origin_output) != 0) {
        report_bug();
    }
    free(output_buf);

}

void test_body(const char *test_file) {
    sem_check(test_file);
    char input_file[1024];
    sprintf(input_file, "%s.input", test_file);

    memset(inserted_pos, 0, sizeof(inserted_pos));
    memset(refactor_pos, 0, sizeof(refactor_pos));
    memset(coverage_map, 0, sizeof(coverage_map));

    visit_file(test_file);
    insert_watch_point(test_file, INSERTED_C_FILE);

    compile_and_check_by_cc(INSERTED_C_FILE);
    char *origin_output = (char *) malloc(sizeof(char) * STDOUT_BUF_SIZ);
    memset(origin_output, 0, sizeof(char) * STDOUT_BUF_SIZ);
    collect_code_cov_with_input(IR_FILE_NAME, origin_output, 1, input_file);

#ifdef RELEASE_MODE
    remove(INSERTED_C_FILE);
    remove(IR_FILE_NAME);
    remove(RW_DECLARED_SOURCE);
#endif
    for (int i = 0; i < ITER_TIMES; i++) {
        random_test(test_file, origin_output, input_file);
#ifdef RELEASE_MODE
        remove(RANDOM_GEN_FILE);
        remove(IR_FILE_NAME);
#endif
    }
    free(origin_output);
}

void test_compiler(const char *compiler_path, const char *test_case_path) {
    DIR *FD;
    if (NULL == (FD = opendir(test_case_path))) {
        fprintf(stderr, "Error : Failed to open input directory\n");
        return;
    }
    struct dirent *in_file;
    char path_buf[PATH_MAX];
    while ((in_file = readdir(FD))) {
        if (!memcmp(in_file->d_name, ".", 1)) {
            continue;
        }
        sprintf(path_buf, "%s/%s", test_case_path, in_file->d_name);
        fprintf(stderr, "%s\n", path_buf);
        test_body(path_buf);
    }
}

int main(int argc, char **argv) {
    srand(233);
    CC_PATH = "./Cmm-Compiler-master/Code/parser";
//    test_body("test_case/bubble-sort.c");
//    test_compiler(CC_PATH, "test_case");
    test_body("test_case/ttttt.c");
    return 0;
}
