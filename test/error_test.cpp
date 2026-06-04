#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"
#include "codefab/assembler/parser.h"
#include "codefab/checker/checker.h"
#include "codefab/executor/executor.h"
#include "codefab/errors.h"
#include <string>
#include <functional>

using namespace codefab;

// ---- Helpers -----------------------------------------------------------------

// 파서 에러를 던지고 메시지에 부분 문자열이 포함되어 있는지 검증
static void expectParserError(const std::string& source,
                              const std::string& expectedSubstr) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        parser.parse();
        FAIL() << "Expected AssemblerError containing: '" << expectedSubstr << "'";
    } catch (const AssemblerError& e) {
        EXPECT_NE(std::string(e.what()).find(expectedSubstr), std::string::npos)
            << "Actual error: " << e.what()
            << "\nExpected to contain: " << expectedSubstr;
    }
}

// 체커 에러를 던지고 메시지에 부분 문자열이 포함되어 있는지 검증
static void expectCheckerError(const std::string& source,
                               const std::string& expectedSubstr) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        Checker checker;
        checker.check(stmts);
        FAIL() << "Expected CheckerError containing: '" << expectedSubstr << "'";
    } catch (const CheckerError& e) {
        EXPECT_NE(std::string(e.what()).find(expectedSubstr), std::string::npos)
            << "Actual error: " << e.what()
            << "\nExpected to contain: " << expectedSubstr;
    }
}

// 런타임 에러를 던지고 메시지에 부분 문자열이 포함되어 있는지 검증
static void expectRuntimeError(const std::string& source,
                               const std::string& expectedSubstr) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        Checker checker;
        checker.check(stmts);
        Executor executor;
        executor.execute(stmts);
        FAIL() << "Expected RuntimeError containing: '" << expectedSubstr << "'";
    } catch (const RuntimeError& e) {
        EXPECT_NE(std::string(e.what()).find(expectedSubstr), std::string::npos)
            << "Actual error: " << e.what()
            << "\nExpected to contain: " << expectedSubstr;
    }
}

// ============================================================================
// 1. 구문 에러 (Parser / Assembler 단계)
// ============================================================================

TEST(ErrorMessageTest, MissingSemicolonAfterPrint) {
    // print 1 + 2   <-- 세미콜론 누락
    expectParserError("print 1 + 2", "Expect ';'");
}

TEST(ErrorMessageTest, MissingSemicolonAfterExpression) {
    // var x = 10  <-- 세미콜론 누락
    expectParserError("var x = 10", "Expect ';'");
}

TEST(ErrorMessageTest, MissingClosingParenAfterExpression) {
    // print (1 + 2;  <-- 닫는 괄호 누락
    expectParserError("print (1 + 2;", "Expect ')' after expression.");
}

TEST(ErrorMessageTest, InvalidAssignmentTarget) {
    // a + b = 3;  <-- 유효하지 않은 대입 대상
    expectParserError(
        "var a = 1;\n"
        "var b = 2;\n"
        "a + b = 3;",
        "Invalid assignment target."
    );
}

TEST(ErrorMessageTest, ExpectExpressionOnUnexpectedToken) {
    // print * 5;  <-- 식이 와야 할 자리에 *
    expectParserError("print * 5;", "Expect expression.");
}

// ============================================================================
// 2. Checker 정적 에러
// ============================================================================

TEST(ErrorMessageTest, SelfReferenceInInitializer) {
    // var a = a;  <-- 자기 초기화식에서 자신을 읽기
    expectCheckerError(
        "{\n"
        "  var a = a;\n"
        "}",
        "Can't read local variable"
    );
}

TEST(ErrorMessageTest, SelfReferenceInInitializerExpression) {
    expectCheckerError(
        "{\n"
        "  var a = a + 1;\n"
        "}",
        "Can't read local variable"
    );
}

TEST(ErrorMessageTest, DuplicateVariableInSameScope) {
    // 같은 스코프에 동일 이름 중복 선언
    expectCheckerError(
        "{\n"
        "  var a = \"hi\";\n"
        "  var a = 3;\n"
        "}",
        "Already a variable with this name in this scope."
    );
}

TEST(ErrorMessageTest, DuplicateVariableInGlobalScope) {
    expectCheckerError(
        "var a = 1;\n"
        "var a = 2;",
        "Already a variable with this name in this scope."
    );
}

// ============================================================================
// 3. 런타임 에러
// ============================================================================

TEST(ErrorMessageTest, UndefinedVariableMessage) {
    // print notDefined;  <-- 정의되지 않은 변수
    expectRuntimeError(
        "print notDefined;",
        "Undefined variable 'notDefined'."
    );
}

TEST(ErrorMessageTest, UndefinedVariableIncludesLineNumber) {
    // 에러 메시지에 줄 번호가 포함되어야 함
    expectRuntimeError(
        "print notDefined;",
        "[line 1]"
    );
}

TEST(ErrorMessageTest, UndefinedVariableOnLine3) {
    expectRuntimeError(
        "var a = 1;\n"
        "var b = 2;\n"
        "print notDefined;",
        "[line 3]"
    );
}

TEST(ErrorMessageTest, PlusOperatorMixedTypes) {
    // 숫자 + 문자열 혼용
    expectRuntimeError(
        "print 1 + \"HI\";",
        "Operands must be two numbers or two strings."
    );
}

TEST(ErrorMessageTest, PlusOperatorStringPlusNumber) {
    // 문자열 + 숫자 혼용
    expectRuntimeError(
        "print \"HI\" + 1;",
        "Operands must be two numbers or two strings."
    );
}

TEST(ErrorMessageTest, UnaryMinusOnString) {
    // 문자열에 단항 마이너스
    expectRuntimeError(
        "print -\"FabCoding\";",
        "Operand must be a number."
    );
}

TEST(ErrorMessageTest, UnaryMinusOnBoolean) {
    // 불리언에 단항 마이너스
    expectRuntimeError(
        "print -true;",
        "Operand must be a number."
    );
}

// ============================================================================
// 유효한 + 연산 (숫자+숫자, 문자열+문자열은 정상 동작)
// ============================================================================

TEST(ErrorMessageTest, PlusNumbersPasses) {
    // 숫자+숫자는 정상
    Lexer lexer("print 1 + 2;");
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto stmts = parser.parse();
    Checker checker;
    ASSERT_NO_THROW(checker.check(stmts));
    Executor executor;
    EXPECT_NO_THROW(executor.execute(stmts));
}

TEST(ErrorMessageTest, PlusStringsPasses) {
    // 문자열+문자열은 정상
    Lexer lexer("print \"hello\" + \" world\";");
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto stmts = parser.parse();
    Checker checker;
    ASSERT_NO_THROW(checker.check(stmts));
    Executor executor;
    EXPECT_NO_THROW(executor.execute(stmts));
}
