cmake_minimum_required(VERSION 3.6)
project(Rigel)

set(CMAKE_C_STANDARD 99)

include_directories("C:/Program Files/LLVM/include")

set(SOURCE_FILES main.c gen_cmm.c gen_cmm.h common.h clang_helper.c clang_helper.h emi.c emi.h)

add_executable(test_clang_ast ${SOURCE_FILES})

link_libraries("C:/Program Files/LLVM/lib")
target_link_libraries(${PROJECT_NAME} clang)