#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"
#include "codefab/assembler/parser.h"
#include "codefab/checker/checker.h"
#include "codefab/executor/executor.h"
#include "codefab/errors.h"
#include <sstream>
#include <iostream>

using namespace codefab;

// ────────────────────────────────────────────────────────────────────────────
// 테스트 픽스처: stdout 캡처 + 매 테스트마다 새 Executor
// ────────────────────────────────────────────────────────────────────────────
class ExecutorTest : public ::testing::Test {
protected:
    std::ostringstream captured;
    std::streambuf* originalCout = nullptr;

    void SetUp() override {
        originalCout = std::cout.rdbuf(captured.rdbuf());
    }

    void TearDown() override {
        std::cout.rdbuf(originalCout);
    }

    // 소스를 전체 파이프라인에 통과시켜 실행
    void run(const std::string& source) {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        Checker checker;
        checker.check(stmts);
        Executor executor;
        executor.execute(stmts);
    }

    // 캡처된 출력에서 줄바꿈 제거한 결과
    std::string output() {
        std::string s = captured.str();
        if (!s.empty() && s.back() == '\n') s.pop_back();
        return s;
    }

    // 런타임 에러를 기대하는 실행
    void runExpectRuntimeError(const std::string& source) {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        Checker checker;
        checker.check(stmts);
        Executor executor;
        EXPECT_THROW(executor.execute(stmts), RuntimeError);
    }
};

// ────────────────────────────────────────────────────────────────────────────
// 리터럴 출력
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, PrintInteger) {
    run("print 42;");
    EXPECT_EQ(output(), "42");
}

TEST_F(ExecutorTest, PrintFloat) {
    run("print 3.14;");
    EXPECT_EQ(output(), "3.14");
}

TEST_F(ExecutorTest, PrintWholeDouble) {
    run("print 10.0;");
    EXPECT_EQ(output(), "10"); // 소수점 없이 출력
}

TEST_F(ExecutorTest, PrintString) {
    run("print \"hello\";");
    EXPECT_EQ(output(), "hello");
}

TEST_F(ExecutorTest, PrintTrue) {
    run("print true;");
    EXPECT_EQ(output(), "true");
}

TEST_F(ExecutorTest, PrintFalse) {
    run("print false;");
    EXPECT_EQ(output(), "false");
}

TEST_F(ExecutorTest, PrintNull) {
    run("print null;");
    EXPECT_EQ(output(), "null");
}

// ────────────────────────────────────────────────────────────────────────────
// 변수 선언 및 참조
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, VarDeclareAndPrint) {
    run("var x = 10; print x;");
    EXPECT_EQ(output(), "10");
}

TEST_F(ExecutorTest, VarAssign) {
    run("var x = 0; x = 42; print x;");
    EXPECT_EQ(output(), "42");
}

TEST_F(ExecutorTest, VarUninitializedIsNull) {
    run("var x; print x;");
    EXPECT_EQ(output(), "null");
}

// ────────────────────────────────────────────────────────────────────────────
// 산술 연산
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, Addition) {
    run("print 3 + 4;");
    EXPECT_EQ(output(), "7");
}

TEST_F(ExecutorTest, Subtraction) {
    run("print 10 - 3;");
    EXPECT_EQ(output(), "7");
}

TEST_F(ExecutorTest, Multiplication) {
    run("print 3 * 4;");
    EXPECT_EQ(output(), "12");
}

TEST_F(ExecutorTest, Division) {
    run("print 10 / 4;");
    EXPECT_EQ(output(), "2.5");
}

TEST_F(ExecutorTest, Modulo) {
    run("print 10 % 3;");
    EXPECT_EQ(output(), "1");
}

TEST_F(ExecutorTest, NegationUnary) {
    run("print -5;");
    EXPECT_EQ(output(), "-5");
}

TEST_F(ExecutorTest, GroupingChangePrecedence) {
    run("print (1 + 2) * 3;");
    EXPECT_EQ(output(), "9");
}

// ────────────────────────────────────────────────────────────────────────────
// 문자열 연결
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, StringConcat) {
    run("print \"hello\" + \" world\";");
    EXPECT_EQ(output(), "hello world");
}

TEST_F(ExecutorTest, StringNumberConcatIsError) {
    // + 연산자는 숫자+숫자 또는 문자열+문자열만 허용
    runExpectRuntimeError("print \"count: \" + 5;");
}

// ────────────────────────────────────────────────────────────────────────────
// 비교 연산
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, GreaterThan) {
    run("print 5 > 3;");
    EXPECT_EQ(output(), "true");
}

TEST_F(ExecutorTest, LessThan) {
    run("print 2 < 3;");
    EXPECT_EQ(output(), "true");
}

TEST_F(ExecutorTest, EqualEqual) {
    run("print 5 == 5;");
    EXPECT_EQ(output(), "true");
}

TEST_F(ExecutorTest, BangEqual) {
    run("print 5 != 3;");
    EXPECT_EQ(output(), "true");
}

// ────────────────────────────────────────────────────────────────────────────
// 논리 연산
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, LogicalAnd) {
    run("print true && false;");
    EXPECT_EQ(output(), "false");
}

TEST_F(ExecutorTest, LogicalOr) {
    run("print false || true;");
    EXPECT_EQ(output(), "true");
}

TEST_F(ExecutorTest, LogicalNot) {
    run("print !true;");
    EXPECT_EQ(output(), "false");
}

TEST_F(ExecutorTest, ShortCircuitAnd) {
    // false && ... 는 오른쪽을 평가하지 않음
    run("var x = false; print x && true;");
    EXPECT_EQ(output(), "false");
}

// ────────────────────────────────────────────────────────────────────────────
// If 문
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, IfThenBranch) {
    run("if (true) print \"yes\";");
    EXPECT_EQ(output(), "yes");
}

TEST_F(ExecutorTest, IfElseBranch) {
    run("if (false) print \"yes\"; else print \"no\";");
    EXPECT_EQ(output(), "no");
}

TEST_F(ExecutorTest, IfWithCondition) {
    run("var x = 10; if (x > 5) print \"big\"; else print \"small\";");
    EXPECT_EQ(output(), "big");
}

// ────────────────────────────────────────────────────────────────────────────
// For 루프
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, ForLoopBasic) {
    run("for (var i = 0; i < 3; i = i + 1) { print i; }");
    EXPECT_EQ(output(), "0\n1\n2");
}

TEST_F(ExecutorTest, ForLoopSum) {
    run(
        "var sum = 0;"
        "for (var i = 1; i <= 5; i = i + 1) {"
        "  sum = sum + i;"
        "}"
        "print sum;"
    );
    EXPECT_EQ(output(), "15");
}

// ────────────────────────────────────────────────────────────────────────────
// 스코프
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, BlockScopeIsolation) {
    // 블록 내 변수는 블록 종료 후 접근 불가
    run(
        "var x = \"outer\";"
        "{"
        "  var x = \"inner\";"
        "  print x;"
        "}"
        "print x;"
    );
    EXPECT_EQ(output(), "inner\nouter");
}

TEST_F(ExecutorTest, OuterVariableAccessFromInner) {
    run(
        "var a = 10;"
        "{"
        "  print a;"
        "}"
    );
    EXPECT_EQ(output(), "10");
}

TEST_F(ExecutorTest, VarShadowingInBlock) {
    // 블록 안의 var 선언은 바깥 변수와 별개(shadowing) — 블록 종료 후 외부 값 유지
    run(
        "var x = \"global\";"
        "{"
        "  var x = \"inner\";"
        "  print x;"
        "}"
        "print x;"
    );
    EXPECT_EQ(output(), "inner\nglobal");
}

TEST_F(ExecutorTest, DanglingElseBindsToNearestIf) {
    // else는 가장 가까운 if에 결합: outer if(true) → inner if(false) → else 실행
    run("if (true) if (false) print \"kfc\"; else print \"bbq\";");
    EXPECT_EQ(output(), "bbq");
}

// ────────────────────────────────────────────────────────────────────────────
// 런타임 에러
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, DivisionByZero) {
    runExpectRuntimeError("var a = 3 / 0;");
}

TEST_F(ExecutorTest, ModuloByZero) {
    runExpectRuntimeError("var a = 5 % 0;");
}

TEST_F(ExecutorTest, UndefinedVariable) {
    runExpectRuntimeError("print z;");
}

TEST_F(ExecutorTest, AssignUndefinedVariable) {
    runExpectRuntimeError("x = 5;");
}

TEST_F(ExecutorTest, TypeMismatchSubtraction) {
    runExpectRuntimeError("print 3 - \"hello\";");
}

TEST_F(ExecutorTest, TypeMismatchMultiplication) {
    runExpectRuntimeError("print true * false;");
}

TEST_F(ExecutorTest, TypeMismatchUnaryMinus) {
    runExpectRuntimeError("print -\"hello\";");
}

// ────────────────────────────────────────────────────────────────────────────
// 복합 시나리오
// ────────────────────────────────────────────────────────────────────────────

TEST_F(ExecutorTest, FibonacciLike) {
    run(
        "var a = 0;"
        "var b = 1;"
        "for (var i = 0; i < 5; i = i + 1) {"
        "  print a;"
        "  var temp = a + b;"
        "  a = b;"
        "  b = temp;"
        "}"
    );
    EXPECT_EQ(output(), "0\n1\n1\n2\n3");
}

TEST_F(ExecutorTest, NestedBlocks) {
    run(
        "var x = 1;"
        "{"
        "  var y = 2;"
        "  {"
        "    var z = 3;"
        "    print x + y + z;"
        "  }"
        "}"
    );
    EXPECT_EQ(output(), "6");
}
