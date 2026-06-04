#pragma once
#include "../ast/stmt.h"
#include "environment.h"
#include "../errors.h"
#include <memory>
#include <vector>
namespace codefab {
class Executor : public ExprVisitor, public StmtVisitor {
public:
    Executor();
    void execute(const std::vector<std::unique_ptr<Stmt>>& statements);
private:
    std::shared_ptr<Environment> m_environment;
    FabValue evaluate(const Expr& e); void executeStmt(const Stmt& s);
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& stmts, std::shared_ptr<Environment> env);
    static bool isTruthy(const FabValue& v); static bool isEqualVal(const FabValue& a, const FabValue& b);
    static void checkNumber(const FabValue& v, const std::string& op);
    static void checkNumbers(const FabValue& l, const FabValue& r, const std::string& op);
    static std::string stringify(const FabValue& v);
    FabValue visitLiteral(const LiteralExpr&) override; FabValue visitVariable(const VariableExpr&) override;
    FabValue visitAssign(const AssignExpr&) override; FabValue visitBinary(const BinaryExpr&) override;
    FabValue visitUnary(const UnaryExpr&) override; FabValue visitLogical(const LogicalExpr&) override;
    FabValue visitGrouping(const GroupingExpr&) override;
    void visitExpression(const ExpressionStmt&) override; void visitPrint(const PrintStmt&) override;
    void visitVarDeclare(const VarDeclareStmt&) override; void visitBlock(const BlockStmt&) override;
    void visitIf(const IfStmt&) override; void visitFor(const ForStmt&) override;
};
} // namespace codefab
