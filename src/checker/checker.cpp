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
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            if (!it->second)
                throw CheckerError("Can't read local variable in its own initializer.");
            return;
        }
    }
}

void Checker::visitExpression(const ExpressionStmt& stmt) { checkExpr(*stmt.expression); }
void Checker::visitPrint(const PrintStmt& stmt)           { checkExpr(*stmt.expression); }

void Checker::visitVarDeclare(const VarDeclareStmt& stmt) {
    declare(stmt.name.origin);
    if (stmt.initializer) checkExpr(*stmt.initializer);
    define(stmt.name.origin);
}

void Checker::visitBlock(const BlockStmt& stmt) {
    beginScope();
    for (const auto& s : stmt.statements) checkStmt(*s);
    endScope();
}

void Checker::visitIf(const IfStmt& stmt) {
    checkExpr(*stmt.condition);
    checkStmt(*stmt.thenBranch);
    if (stmt.elseBranch) checkStmt(*stmt.elseBranch);
}

void Checker::visitFor(const ForStmt& stmt) {
    beginScope();
    if (stmt.initializer) checkStmt(*stmt.initializer);
    if (stmt.condition)   checkExpr(*stmt.condition);
    if (stmt.increment)   checkExpr(*stmt.increment);
    checkStmt(*stmt.body);
    endScope();
}

FabValue Checker::visitLiteral(const LiteralExpr&)          { return nullptr; }
FabValue Checker::visitVariable(const VariableExpr& expr)   { resolveVariable(expr.name.origin); return nullptr; }
FabValue Checker::visitAssign(const AssignExpr& expr)       { checkExpr(*expr.value); return nullptr; }
FabValue Checker::visitBinary(const BinaryExpr& expr)       { checkExpr(*expr.left); checkExpr(*expr.right); return nullptr; }
FabValue Checker::visitUnary(const UnaryExpr& expr)         { checkExpr(*expr.operand); return nullptr; }
FabValue Checker::visitLogical(const LogicalExpr& expr)     { checkExpr(*expr.left); checkExpr(*expr.right); return nullptr; }
FabValue Checker::visitGrouping(const GroupingExpr& expr)   { checkExpr(*expr.expression); return nullptr; }

} // namespace codefab
