#pragma once
#include "executor/executor.h"
#include <string>

namespace codefab {

// 3단계 파이프라인 진입점
// Assembler → Checker → Executor
class CodeFab {
public:
    // 소스 문자열을 파이프라인에 통과시켜 실행
    // executor: 상태를 유지할 실행기 (REPL에서 재사용 가능)
    static void run(const std::string& source, Executor& executor);

    // 파일을 읽어 실행
    static void runFile(const std::string& path);
};

} // namespace codefab
