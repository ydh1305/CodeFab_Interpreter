#pragma once
#include "../ast/stmt.h"
#include "../errors.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace codefab {

class Checker : public ExprVisitor, public StmtVisitor {
public:
    void check(const std::vector<std::unique_ptr<Stmt>>& statements);

private:
    std::vector<std::unordered_map<std::string, bool>> scopes;

    void checkStmt(const Stmt& stmt);
    void checkExpr(const Expr& expr);
    void beginScope();
    void endScope();
    void declare(const std::string& name);
    void define(const std::string& name);
    void resolveVariable(const std::string& name);

    FabValue visitLiteral(const LiteralExpr& expr) override;
    FabValue visitVariable(const VariableExpr& expr) override;
    FabValue visitAssign(const AssignExpr& expr) override;
    FabValue visitBinary(const BinaryExpr& expr) override;
    FabValue visitUnary(const UnaryExpr& expr) override;
    FabValue visitLogical(const LogicalExpr& expr) override;
    FabValue visitGrouping(const GroupingExpr& expr) override;

    void visitExpression(const ExpressionStmt& stmt) override;
    void visitPrint(const PrintStmt& stmt) override;
    void visitVarDeclare(const VarDeclareStmt& stmt) override;
    void visitBlock(const BlockStmt& stmt) override;
    void visitIf(const IfStmt& stmt) override;
    void visitFor(const ForStmt& stmt) override;
};

} // namespace codefab
