#include "codefab/checker/checker.h"

namespace codefab {

// ────────────────────────────────────────────────────────────────────────────
// 공개 인터페이스
// ────────────────────────────────────────────────────────────────────────────

void Checker::check(const std::vector<std::unique_ptr<Stmt>>& statements) {
    beginScope(); // 전역 스코프
    for (const auto& stmt : statements) {
        checkStmt(*stmt);
    }
    endScope();
}

// ────────────────────────────────────────────────────────────────────────────
// 내부 유틸리티
// ────────────────────────────────────────────────────────────────────────────

void Checker::checkStmt(const Stmt& stmt) {
    stmt.accept(*this);
}

void Checker::checkExpr(const Expr& expr) {
    expr.accept(*this);
}

void Checker::beginScope() {
    m_scopes.push_back({});
}

void Checker::endScope() {
    m_scopes.pop_back();
}

// 현재 스코프에 변수를 선언만 함 (초기화 미완료 상태 = false)
void Checker::declare(const std::string& name) {
    if (m_scopes.empty()) return;
    auto& scope = m_scopes.back();
    if (scope.count(name)) {
        throw CheckerError(
            "Already a variable with this name in this scope.");
    }
    scope[name] = false;
}

// 현재 스코프의 변수를 초기화 완료 상태로 변경 (= true)
void Checker::define(const std::string& name) {
    if (m_scopes.empty()) return;
    m_scopes.back()[name] = true;
}

// 가장 안쪽 스코프부터 바깥으로 탐색
// 변수가 선언됐지만 초기화 미완료이면 자기 참조 에러
void Checker::resolveVariable(const std::string& name) {
    for (int i = static_cast<int>(m_scopes.size()) - 1; i >= 0; --i) {
        auto it = m_scopes[i].find(name);
        if (it != m_scopes[i].end()) {
            if (!it->second) {
                throw CheckerError(
                    "Can't read local variable in its own initializer.");
            }
            return; // 정상적으로 발견
        }
    }
    // 스코프에서 찾지 못해도 checker에서는 에러로 처리하지 않음
    // (정의되지 않은 변수 참조는 런타임 에러)
}

// ────────────────────────────────────────────────────────────────────────────
// StmtVisitor 구현
// ────────────────────────────────────────────────────────────────────────────

void Checker::visitExpression(const ExpressionStmt& stmt) {
    checkExpr(*stmt.expression);
}

void Checker::visitPrint(const PrintStmt& stmt) {
    checkExpr(*stmt.expression);
}

void Checker::visitVarDeclare(const VarDeclareStmt& stmt) {
    declare(stmt.name.origin);
    if (stmt.initializer) {
        checkExpr(*stmt.initializer); // 초기화 식 검사 (자기 참조 감지)
    }
    define(stmt.name.origin);
}

void Checker::visitBlock(const BlockStmt& stmt) {
    beginScope();
    for (const auto& s : stmt.statements) {
        checkStmt(*s);
    }
    endScope();
}

void Checker::visitIf(const IfStmt& stmt) {
    checkExpr(*stmt.condition);
    checkStmt(*stmt.thenBranch);
    if (stmt.elseBranch) checkStmt(*stmt.elseBranch);
}

void Checker::visitFor(const ForStmt& stmt) {
    beginScope(); // for 헤더 스코프 (초기화 변수 포함)
    if (stmt.initializer) checkStmt(*stmt.initializer);
    if (stmt.condition)   checkExpr(*stmt.condition);
    if (stmt.increment)   checkExpr(*stmt.increment);
    checkStmt(*stmt.body);
    endScope();
}

// ────────────────────────────────────────────────────────────────────────────
// ExprVisitor 구현 (반환값은 검증 목적이므로 dummy null 사용)
// ────────────────────────────────────────────────────────────────────────────

FabValue Checker::visitLiteral(const LiteralExpr&) {
    return nullptr;
}

FabValue Checker::visitVariable(const VariableExpr& expr) {
    resolveVariable(expr.name.origin);
    return nullptr;
}

FabValue Checker::visitAssign(const AssignExpr& expr) {
    checkExpr(*expr.value);
    return nullptr;
}

FabValue Checker::visitBinary(const BinaryExpr& expr) {
    checkExpr(*expr.left);
    checkExpr(*expr.right);
    return nullptr;
}

FabValue Checker::visitUnary(const UnaryExpr& expr) {
    checkExpr(*expr.operand);
    return nullptr;
}

FabValue Checker::visitLogical(const LogicalExpr& expr) {
    checkExpr(*expr.left);
    checkExpr(*expr.right);
    return nullptr;
}

FabValue Checker::visitGrouping(const GroupingExpr& expr) {
    checkExpr(*expr.expression);
    return nullptr;
}

} // namespace codefab
