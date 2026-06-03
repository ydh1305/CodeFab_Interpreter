#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"
#include "codefab/assembler/parser.h"
#include "codefab/ast/stmt.h"
#include "codefab/errors.h"

using namespace codefab;

// 헬퍼: 소스 → AST
static std::vector<std::unique_ptr<Stmt>> parse(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    return parser.parse();
}

// ────────────────────────────────────────────────────────────────────────────
// 변수 선언
// ────────────────────────────────────────────────────────────────────────────

TEST(ParserTest, VarDeclarationWithInit) {
    auto stmts = parse("var x = 10;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* varDecl = dynamic_cast<VarDeclareStmt*>(stmts[0].get());
    ASSERT_NE(varDecl, nullptr);
    EXPECT_EQ(varDecl->name.origin, "x");
    EXPECT_NE(varDecl->initializer, nullptr);
}

TEST(ParserTest, VarDeclarationWithoutInit) {
    auto stmts = parse("var x;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* varDecl = dynamic_cast<VarDeclareStmt*>(stmts[0].get());
    ASSERT_NE(varDecl, nullptr);
    EXPECT_EQ(varDecl->name.origin, "x");
    EXPECT_EQ(varDecl->initializer, nullptr);
}

TEST(ParserTest, VarDeclarationString) {
    auto stmts = parse("var name = \"fab\";");
    auto* varDecl = dynamic_cast<VarDeclareStmt*>(stmts[0].get());
    ASSERT_NE(varDecl, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(varDecl->initializer.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(std::get<std::string>(lit->value), "fab");
}

// ────────────────────────────────────────────────────────────────────────────
// Print 문
// ────────────────────────────────────────────────────────────────────────────

TEST(ParserTest, PrintStatement) {
    auto stmts = parse("print 42;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* printStmt = dynamic_cast<PrintStmt*>(stmts[0].get());
    ASSERT_NE(printStmt, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(printStmt->expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_DOUBLE_EQ(std::get<double>(lit->value), 42.0);
}

// ────────────────────────────────────────────────────────────────────────────
// If 문
// ────────────────────────────────────────────────────────────────────────────

TEST(ParserTest, IfStatementWithoutElse) {
    auto stmts = parse("if (true) print 1;");
    auto* ifStmt = dynamic_cast<IfStmt*>(stmts[0].get());
    ASSERT_NE(ifStmt, nullptr);
    EXPECT_NE(ifStmt->thenBranch, nullptr);
    EXPECT_EQ(ifStmt->elseBranch, nullptr);
}

TEST(ParserTest, IfStatementWithElse) {
    auto stmts = parse("if (true) print 1; else print 2;");
    auto* ifStmt = dynamic_cast<IfStmt*>(stmts[0].get());
    ASSERT_NE(ifStmt, nullptr);
    EXPECT_NE(ifStmt->thenBranch, nullptr);
    EXPECT_NE(ifStmt->elseBranch, nullptr);
}

// ────────────────────────────────────────────────────────────────────────────
// For 문
// ────────────────────────────────────────────────────────────────────────────

TEST(ParserTest, ForStatement) {
    auto stmts = parse("for (var i = 0; i < 10; i = i + 1) { print i; }");
    auto* forStmt = dynamic_cast<ForStmt*>(stmts[0].get());
    ASSERT_NE(forStmt, nullptr);
    EXPECT_NE(forStmt->initializer, nullptr);
    EXPECT_NE(forStmt->condition, nullptr);
    EXPECT_NE(forStmt->increment, nullptr);
    EXPECT_NE(forStmt->body, nullptr);
}

TEST(ParserTest, ForStatementNoInit) {
    auto stmts = parse("for (; i < 10; i = i + 1) { print i; }");
    auto* forStmt = dynamic_cast<ForStmt*>(stmts[0].get());
    ASSERT_NE(forStmt, nullptr);
    EXPECT_EQ(forStmt->initializer, nullptr);
}

// ────────────────────────────────────────────────────────────────────────────
// 블록
// ────────────────────────────────────────────────────────────────────────────

TEST(ParserTest, BlockStatement) {
    auto stmts = parse("{ var a = 1; print a; }");
    auto* block = dynamic_cast<BlockStmt*>(stmts[0].get());
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->statements.size(), 2u);
}

// ────────────────────────────────────────────────────────────────────────────
// 표현식
// ────────────────────────────────────────────────────────────────────────────

TEST(ParserTest, BinaryExpression) {
    auto stmts = parse("1 + 2;");
    auto* exprStmt = dynamic_cast<ExpressionStmt*>(stmts[0].get());
    ASSERT_NE(exprStmt, nullptr);
    auto* binary = dynamic_cast<BinaryExpr*>(exprStmt->expression.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->op.type, TokenType::PLUS);
}

TEST(ParserTest, UnaryExpression) {
    auto stmts = parse("-5;");
    auto* exprStmt = dynamic_cast<ExpressionStmt*>(stmts[0].get());
    auto* unary = dynamic_cast<UnaryExpr*>(exprStmt->expression.get());
    ASSERT_NE(unary, nullptr);
    EXPECT_EQ(unary->op.type, TokenType::MINUS);
}

TEST(ParserTest, LogicalAndExpression) {
    auto stmts = parse("true && false;");
    auto* exprStmt = dynamic_cast<ExpressionStmt*>(stmts[0].get());
    auto* logical = dynamic_cast<LogicalExpr*>(exprStmt->expression.get());
    ASSERT_NE(logical, nullptr);
    EXPECT_EQ(logical->op.type, TokenType::AND);
}

TEST(ParserTest, LogicalOrExpression) {
    auto stmts = parse("true || false;");
    auto* exprStmt = dynamic_cast<ExpressionStmt*>(stmts[0].get());
    auto* logical = dynamic_cast<LogicalExpr*>(exprStmt->expression.get());
    ASSERT_NE(logical, nullptr);
    EXPECT_EQ(logical->op.type, TokenType::OR);
}

TEST(ParserTest, GroupingExpression) {
    auto stmts = parse("(1 + 2);");
    auto* exprStmt = dynamic_cast<ExpressionStmt*>(stmts[0].get());
    auto* grouping = dynamic_cast<GroupingExpr*>(exprStmt->expression.get());
    ASSERT_NE(grouping, nullptr);
}

TEST(ParserTest, AssignmentExpression) {
    auto stmts = parse("x = 5;");
    auto* exprStmt = dynamic_cast<ExpressionStmt*>(stmts[0].get());
    auto* assign = dynamic_cast<AssignExpr*>(exprStmt->expression.get());
    ASSERT_NE(assign, nullptr);
    EXPECT_EQ(assign->name.origin, "x");
}

// ────────────────────────────────────────────────────────────────────────────
// 파싱 에러
// ────────────────────────────────────────────────────────────────────────────

TEST(ParserTest, MissingSemicolon) {
    EXPECT_THROW(parse("var x = 10"), AssemblerError);
}

TEST(ParserTest, MissingClosingParen) {
    EXPECT_THROW(parse("(1 + 2"), AssemblerError);
}

TEST(ParserTest, MissingClosingBrace) {
    EXPECT_THROW(parse("{ print 1;"), AssemblerError);
}

TEST(ParserTest, InvalidAssignmentTarget) {
    EXPECT_THROW(parse("1 + 2 = 3;"), AssemblerError);
}

// ────────────────────────────────────────────────────────────────────────────
// 연산자 우선순위
// ────────────────────────────────────────────────────────────────────────────

TEST(ParserTest, OperatorPrecedence) {
    // 1 + 2 * 3 는 1 + (2 * 3)으로 파싱되어야 함
    auto stmts = parse("1 + 2 * 3;");
    auto* exprStmt = dynamic_cast<ExpressionStmt*>(stmts[0].get());
    auto* add = dynamic_cast<BinaryExpr*>(exprStmt->expression.get());
    ASSERT_NE(add, nullptr);
    EXPECT_EQ(add->op.type, TokenType::PLUS);
    // 오른쪽이 곱셈이어야 함
    auto* mul = dynamic_cast<BinaryExpr*>(add->right.get());
    ASSERT_NE(mul, nullptr);
    EXPECT_EQ(mul->op.type, TokenType::STAR);
}
