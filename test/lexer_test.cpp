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
