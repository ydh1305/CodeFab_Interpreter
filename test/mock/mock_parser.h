#pragma once
// ============================================================
// Mock Parser Helper
// ------------------------------------------------------------
// Parser가 아직 feature/parser 브랜치에서 개발 중이므로,
// Checker 독립 개발을 위해 AST 노드를 직접 생성하는 헬퍼를 사용.
// feature/parser → tdd-commits 병합 후 real Lexer+Parser로 교체 예정.
// ============================================================

#include "codefab/ast/stmt.h"
#include "codefab/ast/expr.h"
#include "codefab/token.h"
#include <memory>
#include <vector>
#include <string>

namespace codefab::mock {

inline Token makeIdToken(const std::string& name) {
    return Token(TokenType::IDENTIFIER, name);
}

inline std::unique_ptr<Expr> makeLit(double val) {
    return std::make_unique<LiteralExpr>(FabValue{val});
}

inline std::unique_ptr<Expr> makeVar(const std::string& name) {
    return std::make_unique<VariableExpr>(makeIdToken(name));
}

inline std::unique_ptr<Stmt> makeVarDecl(const std::string& name,
                                          std::unique_ptr<Expr> init = nullptr) {
    return std::make_unique<VarDeclareStmt>(makeIdToken(name), std::move(init));
}

inline std::unique_ptr<Stmt> makeBlock(std::vector<std::unique_ptr<Stmt>> stmts) {
    return std::make_unique<BlockStmt>(std::move(stmts));
}

} // namespace codefab::mock
