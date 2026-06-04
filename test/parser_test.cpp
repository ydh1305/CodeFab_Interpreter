#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"   // Lexer PR 병합 후 실제 연동
#include "codefab/assembler/parser.h"
#include "codefab/ast/stmt.h"
#include "codefab/errors.h"

using namespace codefab;

// Lexer PR #1 병합 완료 → Mock Token 제거, 실제 Lexer 파이프라인으로 전환
static std::vector<std::unique_ptr<Stmt>> parse(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    return parser.parse();
}

TEST(ParserTest, VarDeclarationWithInit) {
    auto stmts = parse("var x = 10;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* v = dynamic_cast<VarDeclareStmt*>(stmts[0].get());
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->name.origin, "x");
    EXPECT_NE(v->initializer, nullptr);
}

TEST(ParserTest, VarDeclarationWithoutInit) {
    auto stmts = parse("var x;");
    auto* v = dynamic_cast<VarDeclareStmt*>(stmts[0].get());
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(v->initializer, nullptr);
}

TEST(ParserTest, PrintStatement) {
    auto stmts = parse("print 42;");
    auto* p = dynamic_cast<PrintStmt*>(stmts[0].get());
    ASSERT_NE(p, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(p->expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_DOUBLE_EQ(std::get<double>(lit->value), 42.0);
}

TEST(ParserTest, IfStatementWithElse) {
    auto stmts = parse("if (true) print 1; else print 2;");
    auto* s = dynamic_cast<IfStmt*>(stmts[0].get());
    ASSERT_NE(s, nullptr);
    EXPECT_NE(s->thenBranch, nullptr);
    EXPECT_NE(s->elseBranch, nullptr);
}

TEST(ParserTest, ForStatement) {
    auto stmts = parse("for (var i = 0; i < 10; i = i + 1) { print i; }");
    auto* s = dynamic_cast<ForStmt*>(stmts[0].get());
    ASSERT_NE(s, nullptr);
    EXPECT_NE(s->initializer, nullptr);
    EXPECT_NE(s->condition, nullptr);
    EXPECT_NE(s->increment, nullptr);
}

TEST(ParserTest, BlockStatement) {
    auto stmts = parse("{ var a = 1; print a; }");
    auto* b = dynamic_cast<BlockStmt*>(stmts[0].get());
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->statements.size(), 2u);
}

TEST(ParserTest, BinaryExpression) {
    auto stmts = parse("1 + 2;");
    auto* e = dynamic_cast<ExpressionStmt*>(stmts[0].get());
    ASSERT_NE(e, nullptr);
    auto* b = dynamic_cast<BinaryExpr*>(e->expression.get());
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->op.type, TokenType::PLUS);
}

TEST(ParserTest, OperatorPrecedence) {
    auto stmts = parse("1 + 2 * 3;");
    auto* e = dynamic_cast<ExpressionStmt*>(stmts[0].get());
    auto* add = dynamic_cast<BinaryExpr*>(e->expression.get());
    ASSERT_NE(add, nullptr);
    EXPECT_EQ(add->op.type, TokenType::PLUS);
    auto* mul = dynamic_cast<BinaryExpr*>(add->right.get());
    ASSERT_NE(mul, nullptr);
    EXPECT_EQ(mul->op.type, TokenType::STAR);
}

TEST(ParserTest, MissingSemicolon) {
    EXPECT_THROW(parse("var x = 10"), AssemblerError);
}

TEST(ParserTest, InvalidAssignmentTarget) {
    EXPECT_THROW(parse("1 + 2 = 3;"), AssemblerError);
}
