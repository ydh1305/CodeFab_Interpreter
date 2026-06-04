#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"
#include "codefab/assembler/parser.h"
#include "codefab/checker/checker.h"
#include "codefab/errors.h"
using namespace codefab;

// Parser PR #4~#7 병합 완료 -> Mock AST 제거, 실제 파이프라인으로 전환
// parseAndCheck(): Parsing + Semantic analysis 분리 → 단일 책임
static std::vector<std::unique_ptr<Stmt>> parseToStmts(const std::string& src) {
    Lexer l(src); auto t = l.tokenize();
    Parser p(std::move(t)); return p.parse();
}
static void checkSource(const std::string& source) {
    auto stmts = parseToStmts(source);
    Checker checker; checker.check(stmts);
}

TEST(CheckerTest, ValidVarDeclaration)    { EXPECT_NO_THROW(checkSource("var a = 10;")); }
TEST(CheckerTest, ValidVarReference)      { EXPECT_NO_THROW(checkSource("var a = 10; print a;")); }
TEST(CheckerTest, ValidOuterScopeReference){ EXPECT_NO_THROW(checkSource("var x=5; var y=x+1;")); }
TEST(CheckerTest, ValidShadowingInInnerScope){ EXPECT_NO_THROW(checkSource("var a=1; { var a=2; }")); }
TEST(CheckerTest, ValidIfStatement)       { EXPECT_NO_THROW(checkSource("var x=10; if(x>5){print \"big\";} else{print \"small\";}")); }
TEST(CheckerTest, ValidForLoop)           { EXPECT_NO_THROW(checkSource("for(var i=0;i<3;i=i+1){print i;}")); }
TEST(CheckerTest, DuplicateVarInGlobalScope){ EXPECT_THROW(checkSource("var a=1; var a=2;"), CheckerError); }
TEST(CheckerTest, DuplicateVarInBlock)    { EXPECT_THROW(checkSource("{ var b=1; var b=2; }"), CheckerError); }
TEST(CheckerTest, ForLoopVarShadowsOuter) { EXPECT_NO_THROW(checkSource("var i=0; for(var i=0;i<3;i=i+1){print i;}")); }
TEST(CheckerTest, DuplicateVarInBlockConfirmed){ EXPECT_THROW(checkSource("{ var x=1; var x=2; }"), CheckerError); }
TEST(CheckerTest, SelfReferenceInInit)    { EXPECT_THROW(checkSource("var a=a;"), CheckerError); }
TEST(CheckerTest, SelfReferenceInInitExpression){ EXPECT_THROW(checkSource("var a=a+1;"), CheckerError); }
TEST(CheckerTest, SelfReferenceInInitComplex){ EXPECT_THROW(checkSource("var x=(x+3)*2;"), CheckerError); }
TEST(CheckerTest, SelfReferenceInInnerScope){ EXPECT_THROW(checkSource("var a=1; { var a=a+1; }"), CheckerError); }
TEST(CheckerTest, VariableFromOuterScopeIsOk){ EXPECT_NO_THROW(checkSource("var outer=10; var inner=outer+5;")); }
