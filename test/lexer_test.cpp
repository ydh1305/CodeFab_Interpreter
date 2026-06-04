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
