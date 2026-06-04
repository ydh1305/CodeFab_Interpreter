#include <gtest/gtest.h>
#include "codefab/checker/checker.h"
#include "codefab/errors.h"
#include "mock/mock_parser.h"   // Parser 미병합으로 AST를 직접 생성하여 테스트

using namespace codefab;

// Parser가 feature/parser 브랜치에서 개발 중이므로 Mock AST로 대체
static void checkWith(std::vector<std::unique_ptr<Stmt>> stmts) {
    Checker checker;
    checker.check(stmts);
}

TEST(CheckerTest, ValidVarDeclaration_MockAST) {
    std::vector<std::unique_ptr<Stmt>> stmts;
    stmts.push_back(mock::makeVarDecl("a", mock::makeLit(10.0)));
    EXPECT_NO_THROW(checkWith(std::move(stmts)));
}

TEST(CheckerTest, DuplicateVarInScope_MockAST) {
    std::vector<std::unique_ptr<Stmt>> stmts;
    stmts.push_back(mock::makeVarDecl("a"));
    stmts.push_back(mock::makeVarDecl("a")); // 동일 스코프 중복
    EXPECT_THROW(checkWith(std::move(stmts)), CheckerError);
}

TEST(CheckerTest, SelfReferenceInInit_MockAST) {
    // var a = a; — 자기 참조
    std::vector<std::unique_ptr<Stmt>> stmts;
    stmts.push_back(mock::makeVarDecl("a", mock::makeVar("a")));
    EXPECT_THROW(checkWith(std::move(stmts)), CheckerError);
}

TEST(CheckerTest, ValidShadowingInBlock_MockAST) {
    std::vector<std::unique_ptr<Stmt>> outer;
    outer.push_back(mock::makeVarDecl("a", mock::makeLit(1.0)));
    std::vector<std::unique_ptr<Stmt>> inner;
    inner.push_back(mock::makeVarDecl("a", mock::makeLit(2.0))); // 다른 스코프 → OK
    outer.push_back(mock::makeBlock(std::move(inner)));
    EXPECT_NO_THROW(checkWith(std::move(outer)));
}

TEST(CheckerTest, DuplicateVarInBlock_MockAST) {
    std::vector<std::unique_ptr<Stmt>> inner;
    inner.push_back(mock::makeVarDecl("b"));
    inner.push_back(mock::makeVarDecl("b")); // 같은 블록 → 에러
    std::vector<std::unique_ptr<Stmt>> stmts;
    stmts.push_back(mock::makeBlock(std::move(inner)));
    EXPECT_THROW(checkWith(std::move(stmts)), CheckerError);
}
