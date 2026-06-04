#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"
#include "codefab/errors.h"

using namespace codefab;

// 헬퍼: 소스를 토큰화하고 EOF 제외한 목록 반환
static std::vector<Token> lex(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    tokens.pop_back(); // EOF 제거
    return tokens;
}

// ────────────────────────────────────────────────────────────────────────────
// 기본 리터럴 토큰
// ────────────────────────────────────────────────────────────────────────────

TEST(LexerTest, IntegerLiteral) {
    auto tokens = lex("42");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[0].origin, "42");
}

TEST(LexerTest, FloatLiteral) {
    auto tokens = lex("3.14");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[0].origin, "3.14");
}

TEST(LexerTest, StringLiteral) {
    auto tokens = lex("\"hello world\"");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::STRING);
    EXPECT_EQ(tokens[0].origin, "hello world"); // 따옴표 미포함
}

TEST(LexerTest, TrueLiteral) {
    auto tokens = lex("true");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::TRUE_TOKEN);
}

TEST(LexerTest, FalseLiteral) {
    auto tokens = lex("false");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::FALSE_TOKEN);
}

TEST(LexerTest, NullLiteral) {
    auto tokens = lex("null");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::NULL_TOKEN);
}

// ────────────────────────────────────────────────────────────────────────────
// 키워드
// ────────────────────────────────────────────────────────────────────────────

TEST(LexerTest, Keywords) {
    auto tokens = lex("var print if else for");
    ASSERT_EQ(tokens.size(), 5u);
    EXPECT_EQ(tokens[0].type, TokenType::VAR);
    EXPECT_EQ(tokens[1].type, TokenType::PRINT);
    EXPECT_EQ(tokens[2].type, TokenType::IF);
    EXPECT_EQ(tokens[3].type, TokenType::ELSE);
    EXPECT_EQ(tokens[4].type, TokenType::FOR);
}

// ────────────────────────────────────────────────────────────────────────────
// 식별자
// ────────────────────────────────────────────────────────────────────────────

TEST(LexerTest, Identifier) {
    auto tokens = lex("myVariable");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].origin, "myVariable");
}

TEST(LexerTest, IdentifierWithUnderscore) {
    auto tokens = lex("_count_2");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
}

// ────────────────────────────────────────────────────────────────────────────
// 연산자 및 구분자
// ────────────────────────────────────────────────────────────────────────────

TEST(LexerTest, ArithmeticOperators) {
    auto tokens = lex("+ - * / %");
    ASSERT_EQ(tokens.size(), 5u);
    EXPECT_EQ(tokens[0].type, TokenType::PLUS);
    EXPECT_EQ(tokens[1].type, TokenType::MINUS);
    EXPECT_EQ(tokens[2].type, TokenType::STAR);
    EXPECT_EQ(tokens[3].type, TokenType::SLASH);
    EXPECT_EQ(tokens[4].type, TokenType::PERCENT);
}

TEST(LexerTest, ComparisonOperators) {
    auto tokens = lex("> >= < <= == !=");
    ASSERT_EQ(tokens.size(), 6u);
    EXPECT_EQ(tokens[0].type, TokenType::GREATER);
    EXPECT_EQ(tokens[1].type, TokenType::GREATER_EQUAL);
    EXPECT_EQ(tokens[2].type, TokenType::LESS);
    EXPECT_EQ(tokens[3].type, TokenType::LESS_EQUAL);
    EXPECT_EQ(tokens[4].type, TokenType::EQUAL_EQUAL);
    EXPECT_EQ(tokens[5].type, TokenType::BANG_EQUAL);
}

TEST(LexerTest, LogicalOperators) {
    auto tokens = lex("&& ||");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].type, TokenType::AND);
    EXPECT_EQ(tokens[1].type, TokenType::OR);
}

TEST(LexerTest, AssignmentOperator) {
    auto tokens = lex("=");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::EQUAL);
}

TEST(LexerTest, Delimiters) {
    auto tokens = lex("; { } ( )");
    ASSERT_EQ(tokens.size(), 5u);
    EXPECT_EQ(tokens[0].type, TokenType::SEMICOLON);
    EXPECT_EQ(tokens[1].type, TokenType::LEFT_BRACE);
    EXPECT_EQ(tokens[2].type, TokenType::RIGHT_BRACE);
    EXPECT_EQ(tokens[3].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(tokens[4].type, TokenType::RIGHT_PAREN);
}

// ────────────────────────────────────────────────────────────────────────────
// 공백 및 주석 처리
// ────────────────────────────────────────────────────────────────────────────

TEST(LexerTest, WhitespaceIgnored) {
    auto tokens = lex("  42  ");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].type, TokenType::NUMBER);
}

TEST(LexerTest, CommentIgnored) {
    auto tokens = lex("42 // this is a comment\n99");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].origin, "42");
    EXPECT_EQ(tokens[1].origin, "99");
}

TEST(LexerTest, MultilineCode) {
    auto tokens = lex("var\nx\n=\n10\n;");
    ASSERT_EQ(tokens.size(), 5u);
    EXPECT_EQ(tokens[0].type, TokenType::VAR);
    EXPECT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[2].type, TokenType::EQUAL);
    EXPECT_EQ(tokens[3].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[4].type, TokenType::SEMICOLON);
}

// ────────────────────────────────────────────────────────────────────────────
// EOF 토큰
// ────────────────────────────────────────────────────────────────────────────

TEST(LexerTest, AlwaysEndsWithEOF) {
    Lexer lexer("42");
    auto tokens = lexer.tokenize();
    EXPECT_EQ(tokens.back().type, TokenType::EOF_TOKEN);
}

// ────────────────────────────────────────────────────────────────────────────
// 에러 케이스
// ────────────────────────────────────────────────────────────────────────────

TEST(LexerTest, UnterminatedString) {
    EXPECT_THROW(lex("\"hello"), AssemblerError);
}

TEST(LexerTest, UnexpectedCharacter) {
    EXPECT_THROW(lex("@"), AssemblerError);
}

TEST(LexerTest, SingleAmpersandError) {
    EXPECT_THROW(lex("&"), AssemblerError);
}

TEST(LexerTest, SinglePipeError) {
    EXPECT_THROW(lex("|"), AssemblerError);
}

// ────────────────────────────────────────────────────────────────────────────
// 복합 표현식 토큰화
// ────────────────────────────────────────────────────────────────────────────

TEST(LexerTest, ComplexExpression) {
    auto tokens = lex("x + 3 * 2");
    ASSERT_EQ(tokens.size(), 5u);
    EXPECT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[1].type, TokenType::PLUS);
    EXPECT_EQ(tokens[2].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[3].type, TokenType::STAR);
    EXPECT_EQ(tokens[4].type, TokenType::NUMBER);
}
