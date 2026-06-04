#pragma once
#include "expr.h"
#include <vector>
#include <memory>

namespace codefab {

// 전방 선언
class StmtVisitor;

// ────────────────────────────────────────────────────────────────────────────
// Stmt 기반 클래스
// 값 반환 없이 동작을 수행하는 코드 단위
// ────────────────────────────────────────────────────────────────────────────
class Stmt {
public:
    virtual void accept(StmtVisitor& visitor) const = 0;
    virtual ~Stmt() = default;
};

// ────────────────────────────────────────────────────────────────────────────
// 구체적인 Stmt 노드
// ────────────────────────────────────────────────────────────────────────────

// Expr을 감싸는 문장 wrapper
struct ExpressionStmt : public Stmt {
    std::unique_ptr<Expr> expression;
    explicit ExpressionStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
    void accept(StmtVisitor& visitor) const override;
};

// 출력 문장
struct PrintStmt : public Stmt {
    std::unique_ptr<Expr> expression;
    explicit PrintStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
    void accept(StmtVisitor& visitor) const override;
};

// 변수 선언 (var name = initializer;)
struct VarDeclareStmt : public Stmt {
    Token name;
    std::unique_ptr<Expr> initializer; // nullptr이면 초기화 없음
    VarDeclareStmt(Token name, std::unique_ptr<Expr> init)
        : name(std::move(name)), initializer(std::move(init)) {}
    void accept(StmtVisitor& visitor) const override;
};

// 블록 문장 { ... }
struct BlockStmt : public Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
        : statements(std::move(stmts)) {}
    void accept(StmtVisitor& visitor) const override;
};

// if 문
struct IfStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // nullptr이면 else 없음
    IfStmt(std::unique_ptr<Expr> cond,
           std::unique_ptr<Stmt> then_branch,
           std::unique_ptr<Stmt> else_branch)
        : condition(std::move(cond)),
          thenBranch(std::move(then_branch)),
          elseBranch(std::move(else_branch)) {}
    void accept(StmtVisitor& visitor) const override;
};

// for 문
struct ForStmt : public Stmt {
    std::unique_ptr<Stmt> initializer; // nullptr이면 생략
    std::unique_ptr<Expr> condition;   // nullptr이면 무한 루프
    std::unique_ptr<Expr> increment;   // nullptr이면 생략
    std::unique_ptr<Stmt> body;
    ForStmt(std::unique_ptr<Stmt> init,
            std::unique_ptr<Expr> cond,
            std::unique_ptr<Expr> incr,
            std::unique_ptr<Stmt> body)
        : initializer(std::move(init)),
          condition(std::move(cond)),
          increment(std::move(incr)),
          body(std::move(body)) {}
    void accept(StmtVisitor& visitor) const override;
};

// ────────────────────────────────────────────────────────────────────────────
// StmtVisitor 인터페이스
// ────────────────────────────────────────────────────────────────────────────
class StmtVisitor {
public:
    virtual void visitExpression(const ExpressionStmt& stmt) = 0;
    virtual void visitPrint(const PrintStmt& stmt) = 0;
    virtual void visitVarDeclare(const VarDeclareStmt& stmt) = 0;
    virtual void visitBlock(const BlockStmt& stmt) = 0;
    virtual void visitIf(const IfStmt& stmt) = 0;
    virtual void visitFor(const ForStmt& stmt) = 0;
    virtual ~StmtVisitor() = default;
};

// accept 구현
inline void ExpressionStmt::accept(StmtVisitor& v) const { v.visitExpression(*this); }
inline void PrintStmt::accept(StmtVisitor& v) const      { v.visitPrint(*this); }
inline void VarDeclareStmt::accept(StmtVisitor& v) const { v.visitVarDeclare(*this); }
inline void BlockStmt::accept(StmtVisitor& v) const      { v.visitBlock(*this); }
inline void IfStmt::accept(StmtVisitor& v) const         { v.visitIf(*this); }
inline void ForStmt::accept(StmtVisitor& v) const        { v.visitFor(*this); }

} // namespace codefab
