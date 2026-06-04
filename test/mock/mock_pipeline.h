#pragma once
// ============================================================
// Mock Pipeline Helper — Lexer·Parser·Checker 미병합 상태에서
// Executor 독립 개발. 완전 병합 후 실제 파이프라인으로 교체 예정.
// ============================================================
#include "codefab/ast/stmt.h"
#include "codefab/ast/expr.h"
#include "codefab/token.h"
#include <memory>
#include <vector>
#include <string>
namespace codefab::mock {
inline Token mkTok(TokenType t, const std::string& o="", int line=1) { return Token(t,o,line); }
inline std::unique_ptr<Expr> numLit(double v)              { return std::make_unique<LiteralExpr>(FabValue{v}); }
inline std::unique_ptr<Expr> strLit(const std::string& s)  { return std::make_unique<LiteralExpr>(FabValue{s}); }
inline std::unique_ptr<Expr> boolLit(bool b)               { return std::make_unique<LiteralExpr>(FabValue{b}); }
inline std::unique_ptr<Expr> nullLit()                     { return std::make_unique<LiteralExpr>(FabValue{nullptr}); }
inline std::unique_ptr<Expr> varExpr(const std::string& n, int line=1) { return std::make_unique<VariableExpr>(mkTok(TokenType::IDENTIFIER,n,line)); }
inline std::unique_ptr<Expr> binExpr(std::unique_ptr<Expr> l, TokenType op, std::unique_ptr<Expr> r) { return std::make_unique<BinaryExpr>(std::move(l),mkTok(op),std::move(r)); }
inline std::unique_ptr<Stmt> printStmt(std::unique_ptr<Expr> e)  { return std::make_unique<PrintStmt>(std::move(e)); }
inline std::unique_ptr<Stmt> varDecl(const std::string& n, std::unique_ptr<Expr> init=nullptr) { return std::make_unique<VarDeclareStmt>(mkTok(TokenType::IDENTIFIER,n),std::move(init)); }
} // namespace codefab::mock
