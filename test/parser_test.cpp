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
