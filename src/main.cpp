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
// (dangling else: 다음 줄에 else가 올 수 있는지 판단)
static bool stmtHasPendingElse(const codefab::Stmt* stmt) {
    using namespace codefab;
    if (!stmt) return false;
    if (auto* s = dynamic_cast<const IfStmt*>(stmt)) {
        // else가 없으면 이 if가 else를 받을 수 있다
        if (!s->elseBranch) return true;
        // else가 있으면 else 본문의 꼬리 검사
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

// 줄이 'else' 키워드로 시작하는지 확인 (앞 공백 무시, 식별자 경계 확인)
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
    INCOMPLETE,      // EOF에서 파싱 중단 → 더 입력 필요
    PENDING_ELSE,    // 파싱 성공, 마지막 stmt가 else 없는 if → else 대기
    DONE,            // 파싱+검증 성공, 실행 가능
    ERROR_CONSUMED   // 에러 출력 완료 → 누적 버퍼 초기화
};

// 소스를 파싱하고 상태 반환. DONE/PENDING_ELSE이면 stmts에 AST 저장.
// allowPendingElse: 연속 입력 중일 때만 true (단독 한 줄 입력은 false)
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
//
// 세 가지 모드:
//   NORMAL       – "fab> " 프롬프트, 한 줄 입력 즉시 파싱·실행
//   INCOMPLETE   – "... " 프롬프트, EOF 파싱 실패 → 계속 누적
//   PENDING_ELSE – "... " 프롬프트, else 없는 if 파싱 성공 → else 줄 대기
//
// PENDING_ELSE 종료 조건:
//   - 'else'로 시작하는 줄 → 누적 후 재파싱
//   - 빈 줄 또는 else가 아닌 다른 줄 → 현재 누적 실행 후 다음 줄 처리
static void runPromptShell() {
    codefab::Executor executor;
    std::cout << "========================================\n";
    std::cout << "  Fab Language REPL (Code Fab v1.0)\n";
    std::cout << "  종료: 'exit' 또는 Ctrl+D\n";
    std::cout << "========================================\n";

    std::string accumulated;
    int depth = 0;
    ParseState parseState = ParseState::DONE;   // DONE = 정상(초기) 상태
    std::vector<std::unique_ptr<codefab::Stmt>> parsedStmts;

    // 현재 누적 소스를 실행하고 상태를 초기화
    auto flushAndReset = [&]() {
        executeStmts(parsedStmts, executor);
        parsedStmts.clear();
        accumulated.clear();
        depth = 0;
        parseState = ParseState::DONE;
    };

    // 누적 버퍼·상태 리셋 (에러 후 또는 수동)
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
        if (!std::getline(std::cin, line)) break; // EOF

        // ── PENDING_ELSE 처리: else 줄인지 아닌지 판단 ─────────────────────
        if (parseState == ParseState::PENDING_ELSE) {
            if (line.empty()) {
                // 빈 줄 → 사용자가 else 없음을 명시 → 즉시 실행
                flushAndReset();
                continue;
            }
            if (!lineStartsWithElse(line)) {
                // else가 아닌 다음 문장 → 현재 누적 실행 후 이 줄을 새 입력으로
                flushAndReset();
                if (line == "exit" || line == "quit") break;
                // 이 줄을 새 입력처럼 처리 (fall-through)
            }
            // else 줄이면 바로 누적으로 fall-through
        } else {
            // ── 일반/미완성 모드 ────────────────────────────────────────────
            if (!inContinuation) {
                if (line == "exit" || line == "quit") break;
                if (line.empty()) continue;
            }
        }

        // 중괄호 깊이 갱신 & 누적
        depth += braceDepthDelta(line);
        accumulated += line + "\n";

        // 미닫힌 블록이 남아 있으면 계속 읽기
        if (depth > 0) {
            parseState = ParseState::INCOMPLETE;
            continue;
        }

        // depth == 0: 파싱 시도
        // 이미 연속 입력 모드였거나 블록 깊이가 있었으면 pending else 검사 허용
        bool wasContinuation = inContinuation;
        parseState = tryParse(accumulated, parsedStmts, wasContinuation);

        switch (parseState) {
            case ParseState::DONE:
                executeStmts(parsedStmts, executor);
                hardReset();
                break;
            case ParseState::PENDING_ELSE:
                // parsedStmts에 AST 보관, 다음 줄 대기
                break;
            case ParseState::INCOMPLETE:
                // 계속 누적
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
