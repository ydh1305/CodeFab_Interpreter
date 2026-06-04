#pragma once
// ============================================================
// Mock Parser Helper — Parser 미병합 상태에서 Checker 독립 개발
// feature/parser → tdd-commits 병합 후 실제 파이프라인으로 교체 예정
// ============================================================
#include "codefab/ast/stmt.h"
#include "codefab/ast/expr.h"
#include "codefab/token.h"
#include <memory>
#include <vector>
#include <string>
namespace codefab::mock {
inline Token mkId(const std::string& n) { return Token(TokenType::IDENTIFIER, n); }
inline std::unique_ptr<Expr> makeLit(double v) { return std::make_unique<LiteralExpr>(FabValue{v}); }
inline std::unique_ptr<Expr> makeVar(const std::string& n) { return std::make_unique<VariableExpr>(mkId(n)); }
inline std::unique_ptr<Stmt> makeVarDecl(const std::string& n, std::unique_ptr<Expr> init=nullptr) {
    return std::make_unique<VarDeclareStmt>(mkId(n), std::move(init));
}
inline std::unique_ptr<Stmt> makeBlock(std::vector<std::unique_ptr<Stmt>> stmts) {
    return std::make_unique<BlockStmt>(std::move(stmts));
}
} // namespace codefab::mock
