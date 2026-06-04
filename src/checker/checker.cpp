#include "codefab/checker/checker.h"

namespace codefab {

void Checker::check(const std::vector<std::unique_ptr<Stmt>>& statements) {
    beginScope();
    for (const auto& stmt : statements) checkStmt(*stmt);
    endScope();
}

void Checker::checkStmt(const Stmt& stmt) { stmt.accept(*this); }
void Checker::checkExpr(const Expr& expr) { expr.accept(*this); }

void Checker::beginScope() { scopes.push_back({}); }
void Checker::endScope()   { scopes.pop_back(); }

void Checker::declare(const std::string& name) {
    if (scopes.empty()) return;
    auto& scope = scopes.back();
    if (scope.count(name))
        throw CheckerError("Already a variable with this name in this scope.");
    scope[name] = false;
}

void Checker::define(const std::string& name) {
    if (scopes.empty()) return;
    scopes.back()[name] = true;
}

void Checker::resolveVariable(const std::string& name) {
    for (int i = (int)scopes.size() - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            // 자기참조 검사는 다음 커밋에서 추가
            return;
        }
    }
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
