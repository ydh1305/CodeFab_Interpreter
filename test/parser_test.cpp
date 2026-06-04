#include <gtest/gtest.h>
#include "codefab/assembler/parser.h"
#include "codefab/ast/stmt.h"
#include "codefab/errors.h"
#include "mock/mock_lexer.h"   // Lexer 미병합으로 Token을 직접 생성하여 테스트

using namespace codefab;

// Lexer가 feature/lexer 브랜치에서 개발 중이므로 Mock Token으로 대체
static std::vector<std::unique_ptr<Stmt>> parseWith(std::vector<Token> toks) {
    Parser parser(std::move(toks));
    return parser.parse();
}

TEST(ParserTest, VarDeclaration_MockToken) {
    // mock: var x = 10;
    auto stmts = parseWith(mock::makeTokens({
        {TokenType::VAR, "var"}, {TokenType::IDENTIFIER, "x"},
        {TokenType::EQUAL, "="}, {TokenType::NUMBER, "10"},
        {TokenType::SEMICOLON, ";"},
    }));
    ASSERT_EQ(stmts.size(), 1u);
    auto* v = dynamic_cast<VarDeclareStmt*>(stmts[0].get());
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->name.origin, "x");
}

TEST(ParserTest, PrintStatement_MockToken) {
    auto stmts = parseWith(mock::makeTokens({
        {TokenType::PRINT, "print"}, {TokenType::NUMBER, "42"},
        {TokenType::SEMICOLON, ";"},
    }));
    ASSERT_NE(dynamic_cast<PrintStmt*>(stmts[0].get()), nullptr);
}

TEST(ParserTest, BinaryExpression_MockToken) {
    auto stmts = parseWith(mock::makeTokens({
        {TokenType::NUMBER, "1"}, {TokenType::PLUS, "+"},
        {TokenType::NUMBER, "2"}, {TokenType::SEMICOLON, ";"},
    }));
    auto* e = dynamic_cast<ExpressionStmt*>(stmts[0].get());
    ASSERT_NE(e, nullptr);
    auto* b = dynamic_cast<BinaryExpr*>(e->expression.get());
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->op.type, TokenType::PLUS);
}

TEST(ParserTest, IfStatement_MockToken) {
    auto stmts = parseWith(mock::makeTokens({
        {TokenType::IF, "if"}, {TokenType::LEFT_PAREN, "("},
        {TokenType::TRUE_TOKEN, "true"}, {TokenType::RIGHT_PAREN, ")"},
        {TokenType::PRINT, "print"}, {TokenType::NUMBER, "1"},
        {TokenType::SEMICOLON, ";"},
    }));
    ASSERT_NE(dynamic_cast<IfStmt*>(stmts[0].get()), nullptr);
}

TEST(ParserTest, MissingSemicolon_MockToken) {
    EXPECT_THROW(parseWith(mock::makeTokens({
        {TokenType::PRINT, "print"}, {TokenType::NUMBER, "42"},
    })), AssemblerError);
}

TEST(ParserTest, InvalidAssignmentTarget_MockToken) {
    EXPECT_THROW(parseWith(mock::makeTokens({
        {TokenType::NUMBER, "1"}, {TokenType::PLUS, "+"}, {TokenType::NUMBER, "2"},
        {TokenType::EQUAL, "="}, {TokenType::NUMBER, "3"}, {TokenType::SEMICOLON, ";"},
    })), AssemblerError);
}
