#include <gtest/gtest.h>
#include "codefab/executor/executor.h"
#include "codefab/errors.h"
#include "mock/mock_pipeline.h"
#include <sstream>
#include <iostream>
using namespace codefab;

// 전체 파이프라인 미병합 — Mock Statement로 Executor 독립 개발
class ExecutorTest : public ::testing::Test {
protected:
    std::ostringstream cap; std::streambuf* orig = nullptr;
    void SetUp()    override { orig = std::cout.rdbuf(cap.rdbuf()); }
    void TearDown() override { std::cout.rdbuf(orig); }
    std::string out() { auto s=cap.str(); if(!s.empty()&&s.back()=='\n') s.pop_back(); return s; }
    void run(std::vector<std::unique_ptr<Stmt>> stmts) { Executor e; e.execute(stmts); }
};

// ── 리터럴 출력 ──────────────────────────────────────────────────
TEST_F(ExecutorTest, PrintInteger)    { std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::numLit(42.0)));  run(std::move(s)); EXPECT_EQ(out(),"42"); }
TEST_F(ExecutorTest, PrintFloat)      { std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::numLit(3.14))); run(std::move(s)); EXPECT_EQ(out(),"3.14"); }
TEST_F(ExecutorTest, PrintWholeDouble){ std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::numLit(10.0))); run(std::move(s)); EXPECT_EQ(out(),"10"); }
TEST_F(ExecutorTest, PrintString)     { std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::strLit("hello"))); run(std::move(s)); EXPECT_EQ(out(),"hello"); }
TEST_F(ExecutorTest, PrintTrue)       { std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::boolLit(true))); run(std::move(s)); EXPECT_EQ(out(),"true"); }
TEST_F(ExecutorTest, PrintFalse)      { std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::boolLit(false)));run(std::move(s)); EXPECT_EQ(out(),"false"); }
TEST_F(ExecutorTest, PrintNull)       { std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::nullLit()));     run(std::move(s)); EXPECT_EQ(out(),"null"); }

// ── 변수 선언·대입 ───────────────────────────────────────────────
TEST_F(ExecutorTest, VarDeclareAndPrint) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::varDecl("x", mock::numLit(10.0)));
    s.push_back(mock::printStmt(mock::varExpr("x")));
    run(std::move(s)); EXPECT_EQ(out(), "10");
}
TEST_F(ExecutorTest, VarAssign) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::varDecl("x", mock::numLit(0.0)));
    s.push_back(std::make_unique<ExpressionStmt>(
        std::make_unique<AssignExpr>(mock::mkTok(TokenType::IDENTIFIER,"x"), mock::numLit(42.0))));
    s.push_back(mock::printStmt(mock::varExpr("x")));
    run(std::move(s)); EXPECT_EQ(out(), "42");
}
TEST_F(ExecutorTest, VarUninitializedIsNull) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::varDecl("x"));
    s.push_back(mock::printStmt(mock::varExpr("x")));
    run(std::move(s)); EXPECT_EQ(out(), "null");
}
