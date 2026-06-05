// ============================================================
// Debug  빌드 : GoogleTest 전체 실행
// Release 빌드 : Fab 언어 REPL / CLI 실행
// ============================================================

#ifdef _DEBUG
// ── Debug: GoogleTest ────────────────────────────────────────────
#include <gtest/gtest.h>
#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#else
// ── Release: REPL / CLI ──────────────────────────────────────────
#include "codefab/code_fab.h"
#include "codefab/assembler/lexer.h"
#include "codefab/assembler/parser.h"
#include "codefab/checker/checker.h"
#include "codefab/executor/executor.h"
#include "codefab/errors.h"
#include <iostream>
#include <string>
#include <cctype>
#ifdef _WIN32
#include <windows.h>
#endif

// ────────────────────────────────────────────────────────────────────────────
// 유틸리티 함수
// ────────────────────────────────────────────────────────────────────────────

// 한 줄의 중괄호 깊이 변화량 (Lexer 사용 → 문자열 내부 괄호 무시)
static int braceDepthDelta(const std::string& line) {
    try {
        codefab::Lexer lexer(line);
        auto tokens = lexer.tokenize();
        int delta = 0;
        for (const auto& tok : tokens) {
            if (tok.type == codefab::TokenType::LEFT_BRACE)  ++delta;
            else if (tok.type == codefab::TokenType::RIGHT_BRACE) --delta;
        }
        return delta;
    } catch (...) {
        return 0;
    }
}

// 문장 트리에서 "else 없는 if"가 꼬리에 있는지 재귀 검사
static bool stmtHasPendingElse(const codefab::Stmt* stmt) {
    using namespace codefab;
    if (!stmt) return false;
    if (auto* s = dynamic_cast<const IfStmt*>(stmt)) {
        if (!s->elseBranch) return true;
        return stmtHasPendingElse(s->elseBranch.get());
    }
    if (auto* s = dynamic_cast<const BlockStmt*>(stmt)) {
        if (s->statements.empty()) return false;
        return stmtHasPendingElse(s->statements.back().get());
    }
    if (auto* s = dynamic_cast<const ForStmt*>(stmt)) {
        return stmtHasPendingElse(s->body.get());
    }
    return false;
}

static bool hasPendingElse(const std::vector<std::unique_ptr<codefab::Stmt>>& stmts) {
    return !stmts.empty() && stmtHasPendingElse(stmts.back().get());
}

// 줄이 'else' 키워드로 시작하는지 확인
static bool lineStartsWithElse(const std::string& line) {
    size_t i = line.find_first_not_of(" \t");
    if (i == std::string::npos) return false;
    if (line.compare(i, 4, "else") != 0) return false;
    size_t next = i + 4;
    return next >= line.size() ||
           (!std::isalnum(static_cast<unsigned char>(line[next])) && line[next] != '_');
}

// ────────────────────────────────────────────────────────────────────────────
// 파싱 결과 상태
// ────────────────────────────────────────────────────────────────────────────
enum class ParseState {
    INCOMPLETE,
    PENDING_ELSE,
    DONE,
    ERROR_CONSUMED
};

static ParseState tryParse(const std::string& src,
                           std::vector<std::unique_ptr<codefab::Stmt>>& stmts,
                           bool allowPendingElse) {
    stmts.clear();
    try {
        codefab::Lexer lexer(src);
        auto tokens = lexer.tokenize();
        codefab::Parser parser(std::move(tokens));
        stmts = parser.parse();
        codefab::Checker checker;
        checker.check(stmts);
        if (allowPendingElse && hasPendingElse(stmts))
            return ParseState::PENDING_ELSE;
        return ParseState::DONE;
    } catch (const codefab::IncompleteInputError&) {
        return ParseState::INCOMPLETE;
    } catch (const codefab::AssemblerError& e) {
        std::cerr << "[Assembler Error] " << e.what() << "\n";
        return ParseState::ERROR_CONSUMED;
    } catch (const codefab::CheckerError& e) {
        std::cerr << "[Checker Error] " << e.what() << "\n";
        return ParseState::ERROR_CONSUMED;
    }
}

static void executeStmts(const std::vector<std::unique_ptr<codefab::Stmt>>& stmts,
                         codefab::Executor& executor) {
    try {
        executor.execute(stmts);
    } catch (const codefab::RuntimeError& e) {
        std::cerr << "[Runtime Error] " << e.what() << "\n";
    }
}

// ────────────────────────────────────────────────────────────────────────────
// REPL 메인 루프
// ────────────────────────────────────────────────────────────────────────────
static void runPromptShell() {
    codefab::Executor executor;
    std::cout << "========================================\n";
    std::cout << "  Fab Language REPL (Code Fab v1.0)\n";
    std::cout << "  종료: 'exit' 또는 Ctrl+D\n";
    std::cout << "========================================\n";

    std::string accumulated;
    int depth = 0;
    ParseState parseState = ParseState::DONE;
    std::vector<std::unique_ptr<codefab::Stmt>> parsedStmts;

    auto flushAndReset = [&]() {
        executeStmts(parsedStmts, executor);
        parsedStmts.clear();
        accumulated.clear();
        depth = 0;
        parseState = ParseState::DONE;
    };

    auto hardReset = [&]() {
        parsedStmts.clear();
        accumulated.clear();
        depth = 0;
        parseState = ParseState::DONE;
    };

    while (true) {
        bool inContinuation = (depth > 0 ||
                               parseState == ParseState::INCOMPLETE ||
                               parseState == ParseState::PENDING_ELSE);
        std::cout << (inContinuation ? "... " : "fab> ");

        std::string line;
        if (!std::getline(std::cin, line)) break;

        if (parseState == ParseState::PENDING_ELSE) {
            if (line.empty()) { flushAndReset(); continue; }
            if (!lineStartsWithElse(line)) {
                flushAndReset();
                if (line == "exit" || line == "quit") break;
            }
        } else {
            if (!inContinuation) {
                if (line == "exit" || line == "quit") break;
                if (line.empty()) continue;
            }
        }

        depth += braceDepthDelta(line);
        accumulated += line + "\n";

        if (depth > 0) { parseState = ParseState::INCOMPLETE; continue; }

        bool wasContinuation = inContinuation;
        parseState = tryParse(accumulated, parsedStmts, wasContinuation);

        switch (parseState) {
            case ParseState::DONE:
                executeStmts(parsedStmts, executor);
                hardReset();
                break;
            case ParseState::PENDING_ELSE:
                break;
            case ParseState::INCOMPLETE:
                break;
            case ParseState::ERROR_CONSUMED:
                hardReset();
                break;
        }
    }

    std::cout << "\n안녕히 가세요!\n";
}

// ────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (argc == 1) {
        runPromptShell();
    } else if (argc == 2) {
        codefab::CodeFab::runFile(argv[1]);
    } else {
        std::cerr << "사용법: codefab [스크립트 파일]\n";
        return 1;
    }
    return 0;
}

#endif // _DEBUG
