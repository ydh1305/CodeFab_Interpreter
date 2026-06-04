#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"
#include "codefab/errors.h"
using namespace codefab;

static std::vector<Token> lex(const std::string& s) {
    Lexer l(s); auto t = l.tokenize(); t.pop_back(); return t;
}

// ── 숫자·문자열 리터럴 ───────────────────────────────────────────
TEST(LexerTest, IntegerLiteral) {
    auto t = lex("42");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].type, TokenType::NUMBER);
    EXPECT_EQ(t[0].origin, "42");
}
TEST(LexerTest, FloatLiteral) {
    auto t = lex("3.14");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].type, TokenType::NUMBER);
    EXPECT_EQ(t[0].origin, "3.14");
}
TEST(LexerTest, StringLiteral) {
    auto t = lex("\"hello world\"");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].type, TokenType::STRING);
    EXPECT_EQ(t[0].origin, "hello world");
}

// ── 불리언·null·키워드 ───────────────────────────────────────────
TEST(LexerTest, TrueLiteral) {
    auto t = lex("true");
    ASSERT_EQ(t.size(), 1u); EXPECT_EQ(t[0].type, TokenType::TRUE_TOKEN);
}
TEST(LexerTest, FalseLiteral) {
    auto t = lex("false");
    ASSERT_EQ(t.size(), 1u); EXPECT_EQ(t[0].type, TokenType::FALSE_TOKEN);
}
TEST(LexerTest, NullLiteral) {
    auto t = lex("null");
    ASSERT_EQ(t.size(), 1u); EXPECT_EQ(t[0].type, TokenType::NULL_TOKEN);
}
TEST(LexerTest, Keywords) {
    auto t = lex("var print if else for");
    ASSERT_EQ(t.size(), 5u);
    EXPECT_EQ(t[0].type, TokenType::VAR);   EXPECT_EQ(t[1].type, TokenType::PRINT);
    EXPECT_EQ(t[2].type, TokenType::IF);    EXPECT_EQ(t[3].type, TokenType::ELSE);
    EXPECT_EQ(t[4].type, TokenType::FOR);
}

// ── 식별자 ──────────────────────────────────────────────────────
TEST(LexerTest, Identifier) {
    auto t = lex("myVariable");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(t[0].origin, "myVariable");
}
TEST(LexerTest, IdentifierWithUnderscore) {
    auto t = lex("_count_2");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].type, TokenType::IDENTIFIER);
}

// ── 산술·비교 연산자 ──────────────────────────────────────────────
TEST(LexerTest, ArithmeticOperators) {
    auto t = lex("+ - * / %");
    ASSERT_EQ(t.size(), 5u);
    EXPECT_EQ(t[0].type, TokenType::PLUS);  EXPECT_EQ(t[1].type, TokenType::MINUS);
    EXPECT_EQ(t[2].type, TokenType::STAR);  EXPECT_EQ(t[3].type, TokenType::SLASH);
    EXPECT_EQ(t[4].type, TokenType::PERCENT);
}
TEST(LexerTest, ComparisonOperators) {
    auto t = lex("> >= < <= == !=");
    ASSERT_EQ(t.size(), 6u);
    EXPECT_EQ(t[0].type, TokenType::GREATER);       EXPECT_EQ(t[1].type, TokenType::GREATER_EQUAL);
    EXPECT_EQ(t[2].type, TokenType::LESS);          EXPECT_EQ(t[3].type, TokenType::LESS_EQUAL);
    EXPECT_EQ(t[4].type, TokenType::EQUAL_EQUAL);   EXPECT_EQ(t[5].type, TokenType::BANG_EQUAL);
}

TEST(LexerTest, LogicalOperators) {
    auto t = lex("&& ||");
    ASSERT_EQ(t.size(), 2u);
    EXPECT_EQ(t[0].type, TokenType::AND); EXPECT_EQ(t[1].type, TokenType::OR);
}
TEST(LexerTest, AssignmentOperator) {
    auto t = lex("=");
    ASSERT_EQ(t.size(), 1u); EXPECT_EQ(t[0].type, TokenType::EQUAL);
}
TEST(LexerTest, Delimiters) {
    auto t = lex("; { } ( )");
    ASSERT_EQ(t.size(), 5u);
    EXPECT_EQ(t[0].type, TokenType::SEMICOLON);   EXPECT_EQ(t[1].type, TokenType::LEFT_BRACE);
    EXPECT_EQ(t[2].type, TokenType::RIGHT_BRACE); EXPECT_EQ(t[3].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(t[4].type, TokenType::RIGHT_PAREN);
}

// ── 공백·주석·다중줄 ─────────────────────────────────────────────
TEST(LexerTest, WhitespaceIgnored) {
    auto t = lex("  42  ");
    ASSERT_EQ(t.size(), 1u); EXPECT_EQ(t[0].type, TokenType::NUMBER);
}
TEST(LexerTest, CommentIgnored) {
    auto t = lex("42 // this is a comment\n99");
    ASSERT_EQ(t.size(), 2u);
    EXPECT_EQ(t[0].origin, "42"); EXPECT_EQ(t[1].origin, "99");
}
TEST(LexerTest, MultilineCode) {
    auto t = lex("var\nx\n=\n10\n;");
    ASSERT_EQ(t.size(), 5u);
    EXPECT_EQ(t[0].type, TokenType::VAR); EXPECT_EQ(t[1].type, TokenType::IDENTIFIER);
}

// ── EOF 토큰 ─────────────────────────────────────────────────────
TEST(LexerTest, AlwaysEndsWithEOF) {
    Lexer l("42");
    auto t = l.tokenize();
    EXPECT_EQ(t.back().type, TokenType::EOF_TOKEN);
}
