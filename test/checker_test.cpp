#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"   // Parser PR 병합 후 실제 파이프라인 연동
#include "codefab/assembler/parser.h"
#include "codefab/checker/checker.h"
#include "codefab/errors.h"

using namespace codefab;

// Parser PR #2 병합 완료 → Mock AST 제거, 실제 Lexer+Parser 파이프라인으로 전환
static void checkSource(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto stmts = parser.parse();
    Checker checker;
    checker.check(stmts);
}

TEST(CheckerTest, ValidVarDeclaration)      { EXPECT_NO_THROW(checkSource("var a = 10;")); }
TEST(CheckerTest, ValidVarReference)        { EXPECT_NO_THROW(checkSource("var a = 10; print a;")); }
TEST(CheckerTest, ValidShadowingInBlock)    { EXPECT_NO_THROW(checkSource("var a=1; { var a=2; }")); }
TEST(CheckerTest, ValidForLoop)             { EXPECT_NO_THROW(checkSource("for(var i=0;i<3;i=i+1){print i;}")); }

TEST(CheckerTest, DuplicateVarInGlobalScope) {
    EXPECT_THROW(checkSource("var a=1; var a=2;"), CheckerError);
}
TEST(CheckerTest, DuplicateVarInBlock) {
    EXPECT_THROW(checkSource("{ var b=1; var b=2; }"), CheckerError);
}
TEST(CheckerTest, SelfReferenceInInit) {
    EXPECT_THROW(checkSource("var a = a;"), CheckerError);
}
TEST(CheckerTest, SelfReferenceInInitExpression) {
    EXPECT_THROW(checkSource("var a = a + 1;"), CheckerError);
}
TEST(CheckerTest, SelfReferenceInInnerScope) {
    EXPECT_THROW(checkSource("var a=1; { var a = a + 1; }"), CheckerError);
}
TEST(CheckerTest, ForLoopVarShadowsOuter) {
    EXPECT_NO_THROW(checkSource("var i=0; for(var i=0;i<3;i=i+1){print i;}"));
}
