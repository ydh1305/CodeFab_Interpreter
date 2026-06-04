#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"
#include "codefab/assembler/parser.h"
#include "codefab/checker/checker.h"
#include "codefab/executor/executor.h"
#include "codefab/errors.h"
#include <sstream>
#include <iostream>
using namespace codefab;

// 전체 파이프라인 PR #1~#11 병합 완료 -> Mock 제거, 실제 파이프라인으로 전환
class ExecutorTest : public ::testing::Test {
protected:
    std::ostringstream cap; std::streambuf* orig = nullptr;
    void SetUp()    override { orig = std::cout.rdbuf(cap.rdbuf()); }
    void TearDown() override { std::cout.rdbuf(orig); }
    std::string out() { auto s=cap.str(); if(!s.empty()&&s.back()=='\n') s.pop_back(); return s; }
    void run(const std::string& source) {
        Lexer l(source); auto t=l.tokenize();
        Parser p(std::move(t)); auto stmts=p.parse();
        Checker c; c.check(stmts);
        Executor e; e.execute(stmts);
    }
    void runExpectRuntimeError(const std::string& source) {
        Lexer l(source); auto t=l.tokenize();
        Parser p(std::move(t)); auto stmts=p.parse();
        Checker c; c.check(stmts);
        Executor e; EXPECT_THROW(e.execute(stmts), RuntimeError);
    }
};

TEST_F(ExecutorTest, PrintInteger)     { run("print 42;");      EXPECT_EQ(out(),"42"); }
TEST_F(ExecutorTest, PrintFloat)       { run("print 3.14;");    EXPECT_EQ(out(),"3.14"); }
TEST_F(ExecutorTest, PrintWholeDouble) { run("print 10.0;");    EXPECT_EQ(out(),"10"); }
TEST_F(ExecutorTest, PrintString)      { run("print \"hi\";");  EXPECT_EQ(out(),"hi"); }
TEST_F(ExecutorTest, PrintTrue)        { run("print true;");    EXPECT_EQ(out(),"true"); }
TEST_F(ExecutorTest, PrintFalse)       { run("print false;");   EXPECT_EQ(out(),"false"); }
TEST_F(ExecutorTest, PrintNull)        { run("print null;");    EXPECT_EQ(out(),"null"); }
TEST_F(ExecutorTest, VarDeclareAndPrint){ run("var x=10; print x;"); EXPECT_EQ(out(),"10"); }
TEST_F(ExecutorTest, VarAssign)        { run("var x=0; x=42; print x;"); EXPECT_EQ(out(),"42"); }
TEST_F(ExecutorTest, VarUninitializedIsNull){ run("var x; print x;"); EXPECT_EQ(out(),"null"); }
TEST_F(ExecutorTest, Addition)   { run("print 3+4;");    EXPECT_EQ(out(),"7"); }
TEST_F(ExecutorTest, Subtraction){ run("print 10-3;");   EXPECT_EQ(out(),"7"); }
TEST_F(ExecutorTest, Multiplication){ run("print 3*4;"); EXPECT_EQ(out(),"12"); }
TEST_F(ExecutorTest, Division)   { run("print 10/4;");   EXPECT_EQ(out(),"2.5"); }
TEST_F(ExecutorTest, Modulo)     { run("print 10%3;");   EXPECT_EQ(out(),"1"); }
TEST_F(ExecutorTest, NegationUnary){ run("print -5;");   EXPECT_EQ(out(),"-5"); }
TEST_F(ExecutorTest, GroupingChangePrecedence){ run("print (1+2)*3;"); EXPECT_EQ(out(),"9"); }
TEST_F(ExecutorTest, StringConcat){ run("print \"hello\"+\" world\";"); EXPECT_EQ(out(),"hello world"); }
TEST_F(ExecutorTest, StringNumberConcatIsError){ runExpectRuntimeError("print \"count: \"+5;"); }
TEST_F(ExecutorTest, GreaterThan) { run("print 5>3;");   EXPECT_EQ(out(),"true"); }
TEST_F(ExecutorTest, LessThan)    { run("print 2<3;");   EXPECT_EQ(out(),"true"); }
TEST_F(ExecutorTest, EqualEqual)  { run("print 5==5;");  EXPECT_EQ(out(),"true"); }
TEST_F(ExecutorTest, BangEqual)   { run("print 5!=3;");  EXPECT_EQ(out(),"true"); }
TEST_F(ExecutorTest, LogicalAnd)  { run("print true && false;"); EXPECT_EQ(out(),"false"); }
TEST_F(ExecutorTest, LogicalOr)   { run("print false || true;"); EXPECT_EQ(out(),"true"); }
TEST_F(ExecutorTest, LogicalNot)  { run("print !true;");  EXPECT_EQ(out(),"false"); }
TEST_F(ExecutorTest, ShortCircuitAnd){ run("var x=false; print x && true;"); EXPECT_EQ(out(),"false"); }
TEST_F(ExecutorTest, IfThenBranch){ run("if(true) print \"yes\";"); EXPECT_EQ(out(),"yes"); }
TEST_F(ExecutorTest, IfElseBranch){ run("if(false) print \"yes\"; else print \"no\";"); EXPECT_EQ(out(),"no"); }
TEST_F(ExecutorTest, IfWithCondition){ run("var x=10; if(x>5) print \"big\"; else print \"small\";"); EXPECT_EQ(out(),"big"); }
TEST_F(ExecutorTest, ForLoopBasic){ run("for(var i=0;i<3;i=i+1){print i;}"); EXPECT_EQ(out(),"0\n1\n2"); }
TEST_F(ExecutorTest, ForLoopSum)  { run("var sum=0; for(var i=1;i<=5;i=i+1){sum=sum+i;} print sum;"); EXPECT_EQ(out(),"15"); }
TEST_F(ExecutorTest, BlockScopeIsolation){ run("var x=\"outer\"; { var x=\"inner\"; print x; } print x;"); EXPECT_EQ(out(),"inner\nouter"); }
TEST_F(ExecutorTest, OuterVariableAccessFromInner){ run("var a=10; { print a; }"); EXPECT_EQ(out(),"10"); }
TEST_F(ExecutorTest, VarShadowingInBlock){ run("var x=\"global\"; { var x=\"inner\"; print x; } print x;"); EXPECT_EQ(out(),"inner\nglobal"); }
TEST_F(ExecutorTest, DanglingElseBindsToNearestIf){ run("if(true) if(false) print \"kfc\"; else print \"bbq\";"); EXPECT_EQ(out(),"bbq"); }
TEST_F(ExecutorTest, DivisionByZero)    { runExpectRuntimeError("var a=3/0;"); }
TEST_F(ExecutorTest, ModuloByZero)      { runExpectRuntimeError("var a=5%0;"); }
TEST_F(ExecutorTest, UndefinedVariable) { runExpectRuntimeError("print z;"); }
TEST_F(ExecutorTest, AssignUndefinedVariable){ runExpectRuntimeError("x=5;"); }
TEST_F(ExecutorTest, TypeMismatchSubtraction){ runExpectRuntimeError("print 3-\"hello\";"); }
TEST_F(ExecutorTest, TypeMismatchMultiplication){ runExpectRuntimeError("print true*false;"); }
TEST_F(ExecutorTest, TypeMismatchUnaryMinus){ runExpectRuntimeError("print -\"hello\";"); }
TEST_F(ExecutorTest, FibonacciLike){
    run("var a=0; var b=1; for(var i=0;i<5;i=i+1){ print a; var temp=a+b; a=b; b=temp; }");
    EXPECT_EQ(out(),"0\n1\n1\n2\n3");
}
TEST_F(ExecutorTest, NestedBlocks){
    run("var x=1; { var y=2; { var z=3; print x+y+z; } }");
    EXPECT_EQ(out(),"6");
}
