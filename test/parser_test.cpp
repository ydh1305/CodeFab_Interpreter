#include <gtest/gtest.h>
#include "codefab/assembler/lexer.h"
#include "codefab/assembler/parser.h"
#include "codefab/ast/stmt.h"
#include "codefab/errors.h"
using namespace codefab;

// Lexer PR #1~#3 병합 완료 -> Mock Token 제거, 실제 Lexer 파이프라인으로 전환
static std::vector<std::unique_ptr<Stmt>> parse(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    return parser.parse();
}
// 단일 구문 파싱 편의 함수 (테스트 보일러플레이트 감소)
static std::unique_ptr<Stmt> parseOne(const std::string& source) {
    auto stmts = parse(source);
    return stmts.empty() ? nullptr : std::move(stmts[0]);
}
// 단일 구문 파싱 편의 함수 (테스트 보일러플레이트 감소)
static std::unique_ptr<Stmt> parseOne(const std::string& source) {
    auto stmts = parse(source);
    return stmts.empty() ? nullptr : std::move(stmts[0]);
}

TEST(ParserTest, VarDeclarationWithInit) { auto s=parse("var x = 10;"); ASSERT_EQ(s.size(),1u); auto* v=dynamic_cast<VarDeclareStmt*>(s[0].get()); ASSERT_NE(v,nullptr); EXPECT_EQ(v->name.origin,"x"); }
TEST(ParserTest, VarDeclarationWithoutInit) { auto s=parse("var x;"); auto* v=dynamic_cast<VarDeclareStmt*>(s[0].get()); ASSERT_NE(v,nullptr); EXPECT_EQ(v->initializer,nullptr); }
TEST(ParserTest, VarDeclarationString) { auto s=parse("var name = \"fab\";"); auto* v=dynamic_cast<VarDeclareStmt*>(s[0].get()); ASSERT_NE(v,nullptr); auto* lit=dynamic_cast<LiteralExpr*>(v->initializer.get()); ASSERT_NE(lit,nullptr); EXPECT_EQ(std::get<std::string>(lit->value),"fab"); }
TEST(ParserTest, PrintStatement)    { auto s=parse("print 42;"); ASSERT_NE(dynamic_cast<PrintStmt*>(s[0].get()),nullptr); }
TEST(ParserTest, BinaryExpression)  { auto s=parse("1 + 2;"); auto* e=dynamic_cast<ExpressionStmt*>(s[0].get()); auto* b=dynamic_cast<BinaryExpr*>(e->expression.get()); ASSERT_NE(b,nullptr); EXPECT_EQ(b->op.type,TokenType::PLUS); }
TEST(ParserTest, UnaryExpression)   { auto s=parse("-5;"); ASSERT_NE(dynamic_cast<UnaryExpr*>(dynamic_cast<ExpressionStmt*>(s[0].get())->expression.get()),nullptr); }
TEST(ParserTest, GroupingExpression){ auto s=parse("(1+2);"); ASSERT_NE(dynamic_cast<GroupingExpr*>(dynamic_cast<ExpressionStmt*>(s[0].get())->expression.get()),nullptr); }
TEST(ParserTest, LogicalAndExpression){auto s=parse("true && false;"); ASSERT_NE(dynamic_cast<LogicalExpr*>(dynamic_cast<ExpressionStmt*>(s[0].get())->expression.get()),nullptr); }
TEST(ParserTest, LogicalOrExpression) {auto s=parse("true || false;"); ASSERT_NE(dynamic_cast<LogicalExpr*>(dynamic_cast<ExpressionStmt*>(s[0].get())->expression.get()),nullptr); }
TEST(ParserTest, AssignmentExpression){auto s=parse("x = 5;"); ASSERT_NE(dynamic_cast<AssignExpr*>(dynamic_cast<ExpressionStmt*>(s[0].get())->expression.get()),nullptr); }
TEST(ParserTest, IfStatementWithoutElse){auto s=parse("if(true) print 1;"); auto* st=dynamic_cast<IfStmt*>(s[0].get()); ASSERT_NE(st,nullptr); EXPECT_EQ(st->elseBranch,nullptr); }
TEST(ParserTest, IfStatementWithElse){auto s=parse("if(true) print 1; else print 2;"); auto* st=dynamic_cast<IfStmt*>(s[0].get()); ASSERT_NE(st,nullptr); EXPECT_NE(st->elseBranch,nullptr); }
TEST(ParserTest, ForStatement){auto s=parse("for(var i=0;i<10;i=i+1){print i;}"); ASSERT_NE(dynamic_cast<ForStmt*>(s[0].get()),nullptr); }
TEST(ParserTest, BlockStatement){auto s=parse("{ var a=1; print a; }"); auto* b=dynamic_cast<BlockStmt*>(s[0].get()); ASSERT_NE(b,nullptr); EXPECT_EQ(b->statements.size(),2u); }
TEST(ParserTest, MissingSemicolon)       { EXPECT_THROW(parse("var x = 10"),AssemblerError); }
TEST(ParserTest, MissingClosingParen)    { EXPECT_THROW(parse("(1 + 2"),AssemblerError); }
TEST(ParserTest, MissingClosingBrace)    { EXPECT_THROW(parse("{ print 1;"),AssemblerError); }
TEST(ParserTest, InvalidAssignmentTarget){ EXPECT_THROW(parse("1+2=3;"),AssemblerError); }
TEST(ParserTest, OperatorPrecedence){
    auto s=parse("1+2*3;"); auto* e=dynamic_cast<ExpressionStmt*>(s[0].get());
    auto* add=dynamic_cast<BinaryExpr*>(e->expression.get()); ASSERT_NE(add,nullptr); EXPECT_EQ(add->op.type,TokenType::PLUS);
    auto* mul=dynamic_cast<BinaryExpr*>(add->right.get()); ASSERT_NE(mul,nullptr); EXPECT_EQ(mul->op.type,TokenType::STAR);
}
TEST(ParserTest, ForStatementNoInit){auto s=parse("for(;i<10;i=i+1){print i;}"); auto* f=dynamic_cast<ForStmt*>(s[0].get()); ASSERT_NE(f,nullptr); EXPECT_EQ(f->initializer,nullptr); }
