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

// ── 쉐도잉·for 루프 정상 케이스 ─────────────────────────────────
TEST(CheckerTest, ValidShadowingInInnerScope) {
    std::vector<std::unique_ptr<Stmt>> outer;
    outer.push_back(mock::makeVarDecl("a", mock::makeLit(1.0)));
    std::vector<std::unique_ptr<Stmt>> inner;
    inner.push_back(mock::makeVarDecl("a", mock::makeLit(2.0)));
    outer.push_back(mock::makeBlock(std::move(inner)));
    EXPECT_NO_THROW(checkWith(std::move(outer)));
}
TEST(CheckerTest, ValidIfStatement) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::makeVarDecl("x", mock::makeLit(10.0)));
    auto cond = std::make_unique<BinaryExpr>(
        mock::makeVar("x"), Token(TokenType::GREATER, ">"),
        std::make_unique<LiteralExpr>(FabValue{5.0}));
    auto thenB = std::make_unique<PrintStmt>(
        std::make_unique<LiteralExpr>(FabValue{std::string("big")}));
    s.push_back(std::make_unique<IfStmt>(std::move(cond), std::move(thenB), nullptr));
    EXPECT_NO_THROW(checkWith(std::move(s)));
}
