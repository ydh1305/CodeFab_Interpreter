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
    void checkStmt(const Stmt& s); void checkExpr(const Expr& e);
    void beginScope(); void endScope();
    void declare(const std::string& name); void define(const std::string& name);
    void resolveVariable(const std::string& name);
    FabValue visitLiteral(const LiteralExpr&) override; FabValue visitVariable(const VariableExpr&) override;
    FabValue visitAssign(const AssignExpr&) override; FabValue visitBinary(const BinaryExpr&) override;
    FabValue visitUnary(const UnaryExpr&) override; FabValue visitLogical(const LogicalExpr&) override;
    FabValue visitGrouping(const GroupingExpr&) override;
    void visitExpression(const ExpressionStmt&) override; void visitPrint(const PrintStmt&) override;
    void visitVarDeclare(const VarDeclareStmt&) override; void visitBlock(const BlockStmt&) override;
    void visitIf(const IfStmt&) override; void visitFor(const ForStmt&) override;
};
} // namespace codefab
