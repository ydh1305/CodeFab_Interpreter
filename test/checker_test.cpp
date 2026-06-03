#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"
#include "codefab/assembler/parser.h"
#include "codefab/checker/checker.h"
#include "codefab/errors.h"

using namespace codefab;

// 헬퍼: 소스를 파싱하고 Checker 실행
static void checkSource(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto stmts = parser.parse();
    Checker checker;
    checker.check(stmts);
}

// ────────────────────────────────────────────────────────────────────────────
// 정상 케이스 (에러 없음)
// ────────────────────────────────────────────────────────────────────────────

TEST(CheckerTest, ValidVarDeclaration) {
    EXPECT_NO_THROW(checkSource("var a = 10;"));
}

TEST(CheckerTest, ValidVarReference) {
    EXPECT_NO_THROW(checkSource("var a = 10; print a;"));
}

TEST(CheckerTest, ValidShadowingInInnerScope) {
    // 다른 스코프에서의 동일 이름 선언은 허용
    EXPECT_NO_THROW(checkSource(
        "var a = 1;"
        "{"
        "  var a = 2;" // inner scope: OK
        "}"
    ));
}

TEST(CheckerTest, ValidOuterScopeReference) {
    // 외부 스코프 변수를 내부에서 참조
    EXPECT_NO_THROW(checkSource(
        "var x = 5;"
        "var y = x + 1;"
    ));
}

TEST(CheckerTest, ValidForLoop) {
    EXPECT_NO_THROW(checkSource(
        "for (var i = 0; i < 3; i = i + 1) { print i; }"
    ));
}

TEST(CheckerTest, ValidIfStatement) {
    EXPECT_NO_THROW(checkSource(
        "var x = 10;"
        "if (x > 5) { print \"big\"; } else { print \"small\"; }"
    ));
}

// ────────────────────────────────────────────────────────────────────────────
// 에러 케이스 1: 동일 스코프 내 중복 선언
// ────────────────────────────────────────────────────────────────────────────

TEST(CheckerTest, DuplicateVarInGlobalScope) {
    EXPECT_THROW(checkSource(
        "var a = \"first\";"
        "var a = \"second\";"
    ), CheckerError);
}

TEST(CheckerTest, DuplicateVarInBlock) {
    EXPECT_THROW(checkSource(
        "{"
        "  var b = 1;"
        "  var b = 2;"
        "}"
    ), CheckerError);
}

TEST(CheckerTest, ForLoopVarShadowsOuter) {
    // for 헤더는 새 스코프 → 외부 i와 별개이므로 허용
    EXPECT_NO_THROW(checkSource(
        "var i = 0;"
        "for (var i = 0; i < 3; i = i + 1) { print i; }"
    ));
}

TEST(CheckerTest, DuplicateVarInBlockConfirmed) {
    // 같은 블록 내 중복 선언 → 에러
    EXPECT_THROW(checkSource(
        "{"
        "  var x = 1;"
        "  var x = 2;"
        "}"
    ), CheckerError);
}

// ────────────────────────────────────────────────────────────────────────────
// 에러 케이스 2: 초기화 시 자기 참조
// ────────────────────────────────────────────────────────────────────────────

TEST(CheckerTest, SelfReferenceInInit) {
    EXPECT_THROW(checkSource("var a = a;"), CheckerError);
}

TEST(CheckerTest, SelfReferenceInInitExpression) {
    EXPECT_THROW(checkSource("var a = a + 1;"), CheckerError);
}

TEST(CheckerTest, SelfReferenceInInitComplex) {
    EXPECT_THROW(checkSource("var x = (x + 3) * 2;"), CheckerError);
}

TEST(CheckerTest, SelfReferenceInInnerScope) {
    // 내부 스코프에서 같은 이름으로 선언 시 자기 참조
    EXPECT_THROW(checkSource(
        "var a = 1;"
        "{"
        "  var a = a + 1;" // 내부 a는 아직 초기화 안 됨 → 자기 참조 에러
        "}"
    ), CheckerError);
}

// ────────────────────────────────────────────────────────────────────────────
// 스코프 경계 확인
// ────────────────────────────────────────────────────────────────────────────

TEST(CheckerTest, VariableFromOuterScopeIsOk) {
    // 외부 스코프에서 완전히 초기화된 변수는 참조 가능
    EXPECT_NO_THROW(checkSource(
        "var outer = 10;"
        "var inner = outer + 5;" // outer는 이미 정의됨 → OK
    ));
}
