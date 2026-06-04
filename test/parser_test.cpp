#include <gtest/gtest.h>
#include "codefab/assembler/parser.h"
#include "codefab/ast/stmt.h"
#include "codefab/errors.h"
#include "mock/mock_lexer.h"   // Lexer 미병합 — Token 직접 생성
using namespace codefab;

static std::vector<std::unique_ptr<Stmt>> parseWith(std::vector<Token> toks) {
    Parser p(std::move(toks)); return p.parse();
}

// ── 변수 선언 (Mock Token 사용) ─────────────────────────────────
TEST(ParserTest, VarDeclarationWithInit) {
    auto s = parseWith(mock::makeTokens({{TokenType::VAR,"var"},{TokenType::IDENTIFIER,"x"},{TokenType::EQUAL,"="},{TokenType::NUMBER,"10"},{TokenType::SEMICOLON,";"}}));
    ASSERT_EQ(s.size(), 1u);
    auto* v = dynamic_cast<VarDeclareStmt*>(s[0].get());
    ASSERT_NE(v, nullptr); EXPECT_EQ(v->name.origin, "x"); EXPECT_NE(v->initializer, nullptr);
}
TEST(ParserTest, VarDeclarationWithoutInit) {
    auto s = parseWith(mock::makeTokens({{TokenType::VAR,"var"},{TokenType::IDENTIFIER,"x"},{TokenType::SEMICOLON,";"}}));
    auto* v = dynamic_cast<VarDeclareStmt*>(s[0].get());
    ASSERT_NE(v, nullptr); EXPECT_EQ(v->initializer, nullptr);
}
TEST(ParserTest, VarDeclarationString) {
    auto s = parseWith(mock::makeTokens({{TokenType::VAR,"var"},{TokenType::IDENTIFIER,"name"},{TokenType::EQUAL,"="},{TokenType::STRING,"fab"},{TokenType::SEMICOLON,";"}}));
    auto* v = dynamic_cast<VarDeclareStmt*>(s[0].get());
    ASSERT_NE(v, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(v->initializer.get());
    ASSERT_NE(lit, nullptr); EXPECT_EQ(std::get<std::string>(lit->value), "fab");
}

// ── Print·이항 표현식 ───────────────────────────────────────────
TEST(ParserTest, PrintStatement) {
    auto s = parseWith(mock::makeTokens({{TokenType::PRINT,"print"},{TokenType::NUMBER,"42"},{TokenType::SEMICOLON,";"}}));
    ASSERT_NE(dynamic_cast<PrintStmt*>(s[0].get()), nullptr);
}
TEST(ParserTest, BinaryExpression) {
    auto s = parseWith(mock::makeTokens({{TokenType::NUMBER,"1"},{TokenType::PLUS,"+"},{TokenType::NUMBER,"2"},{TokenType::SEMICOLON,";"}}));
    auto* e = dynamic_cast<ExpressionStmt*>(s[0].get());
    ASSERT_NE(e, nullptr);
    auto* b = dynamic_cast<BinaryExpr*>(e->expression.get());
    ASSERT_NE(b, nullptr); EXPECT_EQ(b->op.type, TokenType::PLUS);
}

// ── 단항·그룹 표현식 ─────────────────────────────────────────────
TEST(ParserTest, UnaryExpression) {
    auto s = parseWith(mock::makeTokens({{TokenType::MINUS,"-"},{TokenType::NUMBER,"5"},{TokenType::SEMICOLON,";"}}));
    auto* e = dynamic_cast<ExpressionStmt*>(s[0].get());
    auto* u = dynamic_cast<UnaryExpr*>(e->expression.get());
    ASSERT_NE(u, nullptr); EXPECT_EQ(u->op.type, TokenType::MINUS);
}
TEST(ParserTest, GroupingExpression) {
    auto s = parseWith(mock::makeTokens({{TokenType::LEFT_PAREN,"("},{TokenType::NUMBER,"1"},{TokenType::PLUS,"+"},{TokenType::NUMBER,"2"},{TokenType::RIGHT_PAREN,")"},{TokenType::SEMICOLON,";"}}));
    auto* e = dynamic_cast<ExpressionStmt*>(s[0].get());
    ASSERT_NE(dynamic_cast<GroupingExpr*>(e->expression.get()), nullptr);
}

// ── 논리·대입 표현식 ─────────────────────────────────────────────
TEST(ParserTest, LogicalAndExpression) {
    auto s = parseWith(mock::makeTokens({{TokenType::TRUE_TOKEN,"true"},{TokenType::AND,"&&"},{TokenType::FALSE_TOKEN,"false"},{TokenType::SEMICOLON,";"}}));
    auto* e = dynamic_cast<ExpressionStmt*>(s[0].get());
    ASSERT_NE(dynamic_cast<LogicalExpr*>(e->expression.get()), nullptr);
}
TEST(ParserTest, LogicalOrExpression) {
    auto s = parseWith(mock::makeTokens({{TokenType::TRUE_TOKEN,"true"},{TokenType::OR,"||"},{TokenType::FALSE_TOKEN,"false"},{TokenType::SEMICOLON,";"}}));
    auto* e = dynamic_cast<ExpressionStmt*>(s[0].get());
    ASSERT_NE(dynamic_cast<LogicalExpr*>(e->expression.get()), nullptr);
}
TEST(ParserTest, AssignmentExpression) {
    auto s = parseWith(mock::makeTokens({{TokenType::IDENTIFIER,"x"},{TokenType::EQUAL,"="},{TokenType::NUMBER,"5"},{TokenType::SEMICOLON,";"}}));
    auto* e = dynamic_cast<ExpressionStmt*>(s[0].get());
    ASSERT_NE(dynamic_cast<AssignExpr*>(e->expression.get()), nullptr);
}
