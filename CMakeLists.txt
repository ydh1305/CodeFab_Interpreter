cmake_minimum_required(VERSION 3.14)
project(CodeFab VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC에서 UTF-8 소스 파일 지원
if(MSVC)
    add_compile_options(/utf-8)
endif()

# ── 헤더 파일 목록 ────────────────────────────────────────────────────────────
set(CODEFAB_HEADERS
    include/codefab/token.h
    include/codefab/fab_value.h
    include/codefab/errors.h
    include/codefab/code_fab.h
    include/codefab/ast/expr.h
    include/codefab/ast/stmt.h
    include/codefab/assembler/lexer.h
    include/codefab/assembler/parser.h
    include/codefab/checker/checker.h
    include/codefab/executor/environment.h
    include/codefab/executor/executor.h
)

# ── 소스 파일 목록 ────────────────────────────────────────────────────────────
set(CODEFAB_SOURCES
    src/assembler/lexer.cpp
    src/assembler/parser.cpp
    src/checker/checker.cpp
    src/executor/environment.cpp
    src/executor/executor.cpp
    src/code_fab.cpp
)

# ── 테스트 소스 목록 ──────────────────────────────────────────────────────────
set(TEST_SOURCES
    test/lexer_test.cpp
    test/parser_test.cpp
    test/checker_test.cpp
    test/executor_test.cpp
    test/error_test.cpp
)

# ── 핵심 라이브러리 ───────────────────────────────────────────────────────────
add_library(codefab_lib ${CODEFAB_SOURCES} ${CODEFAB_HEADERS})
target_include_directories(codefab_lib PUBLIC include)

# Solution Explorer 폴더 구조 (헤더)
source_group(
    TREE   "${CMAKE_CURRENT_SOURCE_DIR}/include/codefab"
    PREFIX "Header Files"
    FILES  ${CODEFAB_HEADERS}
)
# Solution Explorer 폴더 구조 (소스)
source_group(
    TREE   "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PREFIX "Source Files"
    FILES  ${CODEFAB_SOURCES}
)

# ── 실행 파일 (REPL + 스크립트 실행) ─────────────────────────────────────────
add_executable(codefab src/main.cpp)
target_link_libraries(codefab PRIVATE codefab_lib)
source_group("Source Files" FILES src/main.cpp)

# ── Google Test (FetchContent) ────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ── 테스트 실행 파일 ──────────────────────────────────────────────────────────
add_executable(codefab_test ${TEST_SOURCES})
target_link_libraries(codefab_test PRIVATE codefab_lib GTest::gtest_main)
target_include_directories(codefab_test PRIVATE include)

# Solution Explorer 폴더 구조 (테스트)
source_group(
    TREE   "${CMAKE_CURRENT_SOURCE_DIR}/test"
    PREFIX "Test Files"
    FILES  ${TEST_SOURCES}
)

include(GoogleTest)
gtest_discover_tests(codefab_test)
