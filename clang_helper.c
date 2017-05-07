//
// Created by Lxiange on 2017/4/24.
//

#include "clang_helper.h"


static bool can_insert_before(enum CXCursorKind kind) {
    switch (kind) {
        case CXCursor_BinaryOperator:
//        case CXCursor_ReturnStmt:// can't delete.
            //todo: two ways: discard warning or ignore return stmt.
        case CXCursor_IfStmt:
        case CXCursor_WhileStmt:
        case CXCursor_CallExpr:
//        case CXCursor_DeclStmt:// todo: can't delete.
            return true;
        default:
            return false;
    }
}


static enum CXChildVisitResult visitor(CXCursor cursor, CXCursor parent /* no use */, CXClientData clientData) {
    CXSourceLocation location = clang_getCursorLocation(cursor);
    if (clang_Location_isFromMainFile(location) == 0)
        return CXChildVisit_Continue;

    enum CXCursorKind cursorKind = clang_getCursorKind(cursor);
    unsigned int line;
    unsigned int column;
    unsigned int offset;
    clang_getSpellingLocation(location, NULL, &line, &column, &offset);
    if (can_insert_before(cursorKind)) {
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

    debug_print("%s error_num:%d\n", test_file, error_num);
    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
    remove(RW_DECLARED_SOURCE);
    return error_num == 0 ? 0 : 1;
}
