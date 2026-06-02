#pragma once
#include "../ast/stmt.h"
#include "environment.h"
#include "../errors.h"
#include <memory>
#include <vector>

namespace codefab {

// Executor Unit: AST를 재귀적으로 평가하여 코드를 실행
class Executor : public ExprVisitor, public StmtVisitor {
public:
    Executor();

    void execute(const std::vector<std::unique_ptr<Stmt>>& statements);

private:
    std::shared_ptr<Environment> environment;

    FabValue evaluate(const Expr& expr);
    void executeStmt(const Stmt& stmt);
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& stmts,
                      std::shared_ptr<Environment> env);

    static bool isTruthy(const FabValue& val);
    static bool isEqualVal(const FabValue& a, const FabValue& b);
    static void checkNumber(const FabValue& val, const std::string& op);
    static void checkNumbers(const FabValue& left, const FabValue& right,
                             const std::string& op);
    static std::string stringify(const FabValue& val);

    // ExprVisitor 구현
    FabValue visitLiteral(const LiteralExpr& expr) override;
    FabValue visitVariable(const VariableExpr& expr) override;
    FabValue visitAssign(const AssignExpr& expr) override;
    FabValue visitBinary(const BinaryExpr& expr) override;
    FabValue visitUnary(const UnaryExpr& expr) override;
    FabValue visitLogical(const LogicalExpr& expr) override;
    FabValue visitGrouping(const GroupingExpr& expr) override;

    // StmtVisitor 구현
    void visitExpression(const ExpressionStmt& stmt) override;
    void visitPrint(const PrintStmt& stmt) override;
    void visitVarDeclare(const VarDeclareStmt& stmt) override;
    void visitBlock(const BlockStmt& stmt) override;
    void visitIf(const IfStmt& stmt) override;
    void visitFor(const ForStmt& stmt) override;
};

} // namespace codefab
