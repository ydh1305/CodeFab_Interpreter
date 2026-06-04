#include <gtest/gtest.h>
#include "codefab/checker/checker.h"
#include "codefab/errors.h"
#include "mock/mock_parser.h"   // Parser 미병합 — AST 직접 생성
using namespace codefab;

static void checkWith(std::vector<std::unique_ptr<Stmt>> stmts) {
    Checker c; c.check(stmts);
}

// ── 정상 케이스: 선언·참조 ──────────────────────────────────────
TEST(CheckerTest, ValidVarDeclaration) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::makeVarDecl("a", mock::makeLit(10.0)));
    EXPECT_NO_THROW(checkWith(std::move(s)));
}
TEST(CheckerTest, ValidVarReference) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::makeVarDecl("a", mock::makeLit(1.0)));
    s.push_back(std::make_unique<PrintStmt>(mock::makeVar("a")));
    EXPECT_NO_THROW(checkWith(std::move(s)));
}
TEST(CheckerTest, ValidOuterScopeReference) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::makeVarDecl("x", mock::makeLit(5.0)));
    s.push_back(mock::makeVarDecl("y", mock::makeVar("x")));
    EXPECT_NO_THROW(checkWith(std::move(s)));
}
