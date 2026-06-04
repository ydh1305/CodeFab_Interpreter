#pragma once
#include "../fab_value.h"
#include "../token.h"
#include <memory>

namespace codefab {

// 전방 선언: ExprVisitor는 아래에서 완전 정의됨
class ExprVisitor;

// ────────────────────────────────────────────────────────────────────────────
// Expr 기반 클래스
// 실행 시 값 하나로 평가되는 코드 단위
// ────────────────────────────────────────────────────────────────────────────
class Expr {
public:
    virtual FabValue accept(ExprVisitor& visitor) const = 0;
    virtual ~Expr() = default;
};

// ────────────────────────────────────────────────────────────────────────────
// 구체적인 Expr 노드 (선언만, accept 구현은 ExprVisitor 정의 이후)
// ────────────────────────────────────────────────────────────────────────────

// 리터럴 값 (숫자, 문자열, true, false, null)
struct LiteralExpr : public Expr {
    FabValue value;
    explicit LiteralExpr(FabValue val) : value(std::move(val)) {}
    FabValue accept(ExprVisitor& visitor) const override;
};

// 변수 참조
struct VariableExpr : public Expr {
    Token name;
    explicit VariableExpr(Token name) : name(std::move(name)) {}
    FabValue accept(ExprVisitor& visitor) const override;
};

// 변수 대입 (name = value)
struct AssignExpr : public Expr {
    Token name;
    std::unique_ptr<Expr> value;
    AssignExpr(Token name, std::unique_ptr<Expr> val)
        : name(std::move(name)), value(std::move(val)) {}
    FabValue accept(ExprVisitor& visitor) const override;
};

// 이항 연산 (left op right)
struct BinaryExpr : public Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    FabValue accept(ExprVisitor& visitor) const override;
};

// 단항 연산 (op operand)
struct UnaryExpr : public Expr {
    Token op;
    std::unique_ptr<Expr> operand;
    UnaryExpr(Token op, std::unique_ptr<Expr> operand)
        : op(std::move(op)), operand(std::move(operand)) {}
    FabValue accept(ExprVisitor& visitor) const override;
};

// 논리 연산 (&& ||) - 단락 평가 지원
struct LogicalExpr : public Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    LogicalExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    FabValue accept(ExprVisitor& visitor) const override;
};

// 괄호 묶음
struct GroupingExpr : public Expr {
    std::unique_ptr<Expr> expression;
    explicit GroupingExpr(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
    FabValue accept(ExprVisitor& visitor) const override;
};

// ────────────────────────────────────────────────────────────────────────────
// ExprVisitor 인터페이스 (모든 Expr 타입 참조 가능 시점에 정의)
// ────────────────────────────────────────────────────────────────────────────
class ExprVisitor {
public:
    virtual FabValue visitLiteral(const LiteralExpr& expr) = 0;
    virtual FabValue visitVariable(const VariableExpr& expr) = 0;
    virtual FabValue visitAssign(const AssignExpr& expr) = 0;
    virtual FabValue visitBinary(const BinaryExpr& expr) = 0;
    virtual FabValue visitUnary(const UnaryExpr& expr) = 0;
    virtual FabValue visitLogical(const LogicalExpr& expr) = 0;
    virtual FabValue visitGrouping(const GroupingExpr& expr) = 0;
    virtual ~ExprVisitor() = default;
};

// ────────────────────────────────────────────────────────────────────────────
// accept 구현 (ExprVisitor 완전 정의 이후 인라인으로 선언)
// ────────────────────────────────────────────────────────────────────────────
inline FabValue LiteralExpr::accept(ExprVisitor& v) const  { return v.visitLiteral(*this); }
inline FabValue VariableExpr::accept(ExprVisitor& v) const { return v.visitVariable(*this); }
inline FabValue AssignExpr::accept(ExprVisitor& v) const   { return v.visitAssign(*this); }
inline FabValue BinaryExpr::accept(ExprVisitor& v) const   { return v.visitBinary(*this); }
inline FabValue UnaryExpr::accept(ExprVisitor& v) const    { return v.visitUnary(*this); }
inline FabValue LogicalExpr::accept(ExprVisitor& v) const  { return v.visitLogical(*this); }
inline FabValue GroupingExpr::accept(ExprVisitor& v) const { return v.visitGrouping(*this); }

} // namespace codefab
