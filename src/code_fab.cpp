#include "codefab/code_fab.h"
#include "codefab/assembler/lexer.h"
#include "codefab/assembler/parser.h"
#include "codefab/checker/checker.h"
#include "codefab/errors.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace codefab {

void CodeFab::run(const std::string& source, Executor& executor) {
    try {
        // 1단계: Assembler - 어휘 분석 (Lexer)
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        // 1단계: Assembler - 구문 분석 (Parser) → AST 생성
        Parser parser(std::move(tokens));
        auto statements = parser.parse();

        // 2단계: Checker - 의미 분석 (중복 선언, 자기 참조 검출)
        Checker checker;
        checker.check(statements);

        // 3단계: Executor - AST 실행
        executor.execute(statements);

    } catch (const AssemblerError& e) {
        std::cerr << "[Assembler Error] " << e.what() << "\n";
    } catch (const CheckerError& e) {
        std::cerr << "[Checker Error] " << e.what() << "\n";
    } catch (const RuntimeError& e) {
        std::cerr << "[Runtime Error] " << e.what() << "\n";
    }
}

void CodeFab::runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "파일을 열 수 없습니다: " << path << "\n";
        return;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    Executor executor;
    run(ss.str(), executor);
}

} // namespace codefab
