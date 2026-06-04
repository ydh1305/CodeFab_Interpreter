#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"   // 전체 파이프라인 PR 병합 후 실제 연동
#include "codefab/assembler/parser.h"
#include "codefab/checker/checker.h"
#include "codefab/executor/executor.h"
#include "codefab/errors.h"
#include <sstream>
#include <iostream>

using namespace codefab;

// 전체 파이프라인(PR #1~#3) 병합 완료 → Mock Statement 제거, 실제 파이프라인으로 전환
class ExecutorTest : public ::testing::Test {
protected:
    std::ostringstream captured;
    std::streambuf* originalCout = nullptr;
    void SetUp()    override { originalCout = std::cout.rdbuf(captured.rdbuf()); }
    void TearDown() override { std::cout.rdbuf(originalCout); }
    std::string output() {
        auto s = captured.str();
        if (!s.empty() && s.back() == '\n') s.pop_back();
        return s;
    }
    void run(const std::string& source) {
        Lexer lexer(source); auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens)); auto stmts = parser.parse();
        Checker checker; checker.check(stmts);
        Executor executor; executor.execute(stmts);
    }
    void runExpectRuntimeError(const std::string& source) {
        Lexer lexer(source); auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens)); auto stmts = parser.parse();
        Checker checker; checker.check(stmts);
        Executor executor;
        EXPECT_THROW(executor.execute(stmts), RuntimeError);
    }
};

TEST_F(ExecutorTest, PrintInteger)     { run("print 42;");       EXPECT_EQ(output(), "42"); }
TEST_F(ExecutorTest, PrintFloat)       { run("print 3.14;");     EXPECT_EQ(output(), "3.14"); }
TEST_F(ExecutorTest, PrintString)      { run("print \"hi\";");   EXPECT_EQ(output(), "hi"); }
TEST_F(ExecutorTest, PrintTrue)        { run("print true;");     EXPECT_EQ(output(), "true"); }
TEST_F(ExecutorTest, PrintNull)        { run("print null;");     EXPECT_EQ(output(), "null"); }

TEST_F(ExecutorTest, VarDeclareAndPrint) { run("var x=10; print x;"); EXPECT_EQ(output(), "10"); }
TEST_F(ExecutorTest, VarAssign)          { run("var x=0; x=42; print x;"); EXPECT_EQ(output(), "42"); }

TEST_F(ExecutorTest, Addition)   { run("print 3+4;");   EXPECT_EQ(output(), "7"); }
TEST_F(ExecutorTest, Subtraction){ run("print 10-3;");  EXPECT_EQ(output(), "7"); }
TEST_F(ExecutorTest, Modulo)     { run("print 10%3;");  EXPECT_EQ(output(), "1"); }
TEST_F(ExecutorTest, StringConcat){ run("print \"a\"+\"b\";"); EXPECT_EQ(output(), "ab"); }

TEST_F(ExecutorTest, IfThenBranch) { run("if (true) print \"yes\";"); EXPECT_EQ(output(), "yes"); }
TEST_F(ExecutorTest, IfElseBranch) { run("if (false) print \"yes\"; else print \"no\";"); EXPECT_EQ(output(), "no"); }

TEST_F(ExecutorTest, ForLoopBasic) {
    run("for(var i=0;i<3;i=i+1){print i;}");
    EXPECT_EQ(output(), "0\n1\n2");
}

TEST_F(ExecutorTest, BlockScopeIsolation) {
    run("var x=\"outer\"; { var x=\"inner\"; print x; } print x;");
    EXPECT_EQ(output(), "inner\nouter");
}

TEST_F(ExecutorTest, DivisionByZero)     { runExpectRuntimeError("var a=3/0;"); }
TEST_F(ExecutorTest, UndefinedVariable)  { runExpectRuntimeError("print z;"); }
TEST_F(ExecutorTest, TypeMismatch)       { runExpectRuntimeError("print 3-\"hi\";"); }
