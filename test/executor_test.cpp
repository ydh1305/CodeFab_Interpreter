#include <gtest/gtest.h>
#include "codefab/executor/executor.h"
#include "codefab/errors.h"
#include "mock/mock_pipeline.h"   // 파이프라인 미병합으로 Statement를 직접 생성하여 테스트
#include <sstream>
#include <iostream>

using namespace codefab;

// Lexer·Parser·Checker가 각 feature 브랜치에서 개발 중이므로 Mock Statement로 대체
class ExecutorMockTest : public ::testing::Test {
protected:
    std::ostringstream captured;
    std::streambuf* original = nullptr;
    void SetUp()    override { original = std::cout.rdbuf(captured.rdbuf()); }
    void TearDown() override { std::cout.rdbuf(original); }
    std::string output() {
        auto s = captured.str();
        if (!s.empty() && s.back() == '\n') s.pop_back();
        return s;
    }
    void run(std::vector<std::unique_ptr<Stmt>> stmts) {
        Executor executor;
        executor.execute(stmts);
    }
};

TEST_F(ExecutorMockTest, PrintNumber) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(mock::numLit(42.0)));
    run(std::move(s));
    EXPECT_EQ(output(), "42");
}

TEST_F(ExecutorMockTest, PrintString) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(mock::strLit("hello")));
    run(std::move(s));
    EXPECT_EQ(output(), "hello");
}

TEST_F(ExecutorMockTest, VarDeclareAndPrint) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::varDecl("x", mock::numLit(10.0)));
    s.push_back(mock::printStmt(mock::varExpr("x")));
    run(std::move(s));
    EXPECT_EQ(output(), "10");
}

TEST_F(ExecutorMockTest, Addition) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(
        mock::binExpr(mock::numLit(3.0), TokenType::PLUS, mock::numLit(4.0))));
    run(std::move(s));
    EXPECT_EQ(output(), "7");
}

TEST_F(ExecutorMockTest, UndefinedVariableThrows) {
    std::vector<std::unique_ptr<Stmt>> s;
    s.push_back(mock::printStmt(mock::varExpr("notDefined", 1)));
    Executor executor;
    EXPECT_THROW(executor.execute(s), RuntimeError);
}
