#include "codefab/checker/checker.h"

namespace codefab {

void Checker::check(const std::vector<std::unique_ptr<Stmt>>& statements) {
    beginScope();
    for (const auto& stmt : statements) checkStmt(*stmt);
    endScope();
}

void Checker::checkStmt(const Stmt& stmt) { stmt.accept(*this); }
void Checker::checkExpr(const Expr& expr) { expr.accept(*this); }

void Checker::beginScope() { m_scopes.push_back({}); }
void Checker::endScope()   { m_scopes.pop_back(); }

void Checker::declare(const std::string& name) {
    if (m_scopes.empty()) return;
    auto& scope = m_scopes.back();
    if (scope.count(name))
        throw CheckerError("Already a variable with this name in this scope.");
    scope[name] = false;
}

void Checker::define(const std::string& name) {
    if (m_scopes.empty()) return;
    m_scopes.back()[name] = true;
}

void Checker::resolveVariable(const std::string& name) {
    for (int i = (int)m_scopes.size() - 1; i >= 0; --i) {
        auto it = m_scopes[i].find(name);
        if (it != m_scopes[i].end()) {
            if (!it->second) // 선언됐으나 초기화 미완료 → 자기 참조
                throw CheckerError("Can't read local variable in its own initializer.");
            return;
        }
    }
    // 스코프에서 미발견: 런타임에서 처리
}

void Checker::visitExpression(const ExpressionStmt& s) { checkExpr(*s.expression); }
void Checker::visitPrint(const PrintStmt& s)           { checkExpr(*s.expression); }

void Checker::visitVarDeclare(const VarDeclareStmt& s) {
    declare(s.name.origin);
    if (s.initializer) checkExpr(*s.initializer);
    define(s.name.origin);
}

void Checker::visitBlock(const BlockStmt& s) {
    beginScope();
    for (const auto& st : s.statements) checkStmt(*st);
    endScope();
}

void Checker::visitIf(const IfStmt& s) {
    checkExpr(*s.condition);
    checkStmt(*s.thenBranch);
    if (s.elseBranch) checkStmt(*s.elseBranch);
}

void Checker::visitFor(const ForStmt& s) {
    beginScope();
    if (s.initializer) checkStmt(*s.initializer);
    if (s.condition)   checkExpr(*s.condition);
    if (s.increment)   checkExpr(*s.increment);
    checkStmt(*s.body);
    endScope();
}

FabValue Checker::visitLiteral(const LiteralExpr&)        { return nullptr; }
FabValue Checker::visitVariable(const VariableExpr& e)    { resolveVariable(e.name.origin); return nullptr; }
FabValue Checker::visitAssign(const AssignExpr& e)        { checkExpr(*e.value); return nullptr; }
FabValue Checker::visitBinary(const BinaryExpr& e)        { checkExpr(*e.left); checkExpr(*e.right); return nullptr; }
FabValue Checker::visitUnary(const UnaryExpr& e)          { checkExpr(*e.operand); return nullptr; }
FabValue Checker::visitLogical(const LogicalExpr& e)      { checkExpr(*e.left); checkExpr(*e.right); return nullptr; }
FabValue Checker::visitGrouping(const GroupingExpr& e)    { checkExpr(*e.expression); return nullptr; }

} // namespace codefab
