#pragma once
#include "../ast/stmt.h"
#include "../errors.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace codefab {

// Checker Unit: DFS로 AST를 순회하며 의미 오류를 검출
// 검출 항목:
//   1. 동일 스코프 내 변수 중복 선언
//   2. 초기화 시 자기 자신 참조 (var a = a + 1;)
class Checker : public ExprVisitor, public StmtVisitor {
public:
    void check(const std::vector<std::unique_ptr<Stmt>>& statements);

private:
    // 스코프 스택: 각 맵의 값이 false이면 선언됐지만 아직 초기화 안 됨
    std::vector<std::unordered_map<std::string, bool>> scopes;

    void checkStmt(const Stmt& stmt);
    void checkExpr(const Expr& expr);

    void beginScope();
    void endScope();
    void declare(const std::string& name);
    void define(const std::string& name);
    void resolveVariable(const std::string& name);

    // ExprVisitor 구현 (검증만, 반환값은 dummy)
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
