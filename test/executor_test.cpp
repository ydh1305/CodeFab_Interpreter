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

// ── 산술·그룹 연산 ───────────────────────────────────────────────
TEST_F(ExecutorTest, Addition)   { std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::binExpr(mock::numLit(3.0),TokenType::PLUS,mock::numLit(4.0)))); run(std::move(s)); EXPECT_EQ(out(),"7"); }
TEST_F(ExecutorTest, Subtraction){ std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::binExpr(mock::numLit(10.0),TokenType::MINUS,mock::numLit(3.0)))); run(std::move(s)); EXPECT_EQ(out(),"7"); }
TEST_F(ExecutorTest, Multiplication){ std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::binExpr(mock::numLit(3.0),TokenType::STAR,mock::numLit(4.0)))); run(std::move(s)); EXPECT_EQ(out(),"12"); }
TEST_F(ExecutorTest, Division)   { std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::binExpr(mock::numLit(10.0),TokenType::SLASH,mock::numLit(4.0)))); run(std::move(s)); EXPECT_EQ(out(),"2.5"); }
TEST_F(ExecutorTest, Modulo)     { std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(mock::binExpr(mock::numLit(10.0),TokenType::PERCENT,mock::numLit(3.0)))); run(std::move(s)); EXPECT_EQ(out(),"1"); }
TEST_F(ExecutorTest, NegationUnary) { std::vector<std::unique_ptr<Stmt>> s; s.push_back(mock::printStmt(std::make_unique<UnaryExpr>(Token(TokenType::MINUS,"-"), mock::numLit(5.0)))); run(std::move(s)); EXPECT_EQ(out(),"-5"); }
TEST_F(ExecutorTest, GroupingChangePrecedence) {
    std::vector<std::unique_ptr<Stmt>> s;
    auto inner = mock::binExpr(mock::numLit(1.0), TokenType::PLUS, mock::numLit(2.0));
    auto grouped = std::make_unique<GroupingExpr>(std::move(inner));
    s.push_back(mock::printStmt(mock::binExpr(std::move(grouped), TokenType::STAR, mock::numLit(3.0))));
    run(std::move(s)); EXPECT_EQ(out(), "9");
}

// ── 문자열·비교 연산 ─────────────────────────────────────────────
TEST_F(ExecutorTest, StringConcat) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(mock::binExpr(mock::strLit("hello"), TokenType::PLUS, mock::strLit(" world"))));
    run(std::move(s)); EXPECT_EQ(out(), "hello world");
}
TEST_F(ExecutorTest, StringNumberConcatIsError) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(mock::binExpr(mock::strLit("count: "), TokenType::PLUS, mock::numLit(5.0))));
    Executor e; EXPECT_THROW(e.execute(s), RuntimeError);
}
TEST_F(ExecutorTest, GreaterThan) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(mock::binExpr(mock::numLit(5.0), TokenType::GREATER, mock::numLit(3.0))));
    run(std::move(s)); EXPECT_EQ(out(), "true");
}
TEST_F(ExecutorTest, LessThan) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(mock::binExpr(mock::numLit(2.0), TokenType::LESS, mock::numLit(3.0))));
    run(std::move(s)); EXPECT_EQ(out(), "true");
}
TEST_F(ExecutorTest, EqualEqual) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(mock::binExpr(mock::numLit(5.0), TokenType::EQUAL_EQUAL, mock::numLit(5.0))));
    run(std::move(s)); EXPECT_EQ(out(), "true");
}
TEST_F(ExecutorTest, BangEqual) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(mock::binExpr(mock::numLit(5.0), TokenType::BANG_EQUAL, mock::numLit(3.0))));
    run(std::move(s)); EXPECT_EQ(out(), "true");
}

// ── 논리 연산·단락 평가 ──────────────────────────────────────────
TEST_F(ExecutorTest, LogicalAnd) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(std::make_unique<LogicalExpr>(mock::boolLit(true), Token(TokenType::AND,"&&"), mock::boolLit(false))));
    run(std::move(s)); EXPECT_EQ(out(), "false");
}
TEST_F(ExecutorTest, LogicalOr) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(std::make_unique<LogicalExpr>(mock::boolLit(false), Token(TokenType::OR,"||"), mock::boolLit(true))));
    run(std::move(s)); EXPECT_EQ(out(), "true");
}
TEST_F(ExecutorTest, LogicalNot) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(std::make_unique<UnaryExpr>(Token(TokenType::BANG,"!"), mock::boolLit(true))));
    run(std::move(s)); EXPECT_EQ(out(), "false");
}
TEST_F(ExecutorTest, ShortCircuitAnd) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::varDecl("x", mock::boolLit(false)));
    s.push_back(mock::printStmt(std::make_unique<LogicalExpr>(mock::varExpr("x"), Token(TokenType::AND,"&&"), mock::boolLit(true))));
    run(std::move(s)); EXPECT_EQ(out(), "false");
}
