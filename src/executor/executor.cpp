#include "codefab/executor/executor.h"
#include "codefab/fab_value.h"
#include <iostream>
#include <cmath>

namespace codefab {

Executor::Executor() : environment(std::make_shared<Environment>()) {}

void Executor::execute(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) executeStmt(*stmt);
}

FabValue Executor::evaluate(const Expr& expr) { return expr.accept(*this); }
void Executor::executeStmt(const Stmt& stmt)  { stmt.accept(*this); }

void Executor::executeBlock(const std::vector<std::unique_ptr<Stmt>>& stmts,
                            std::shared_ptr<Environment> env) {
    auto previous = environment;
    environment = std::move(env);
    try {
        for (const auto& stmt : stmts) executeStmt(*stmt);
    } catch (...) {
        environment = previous;
        throw;
    }
    environment = previous;
}

bool Executor::isTruthy(const FabValue& val)                             { return codefab::isTruthy(val); }
bool Executor::isEqualVal(const FabValue& a, const FabValue& b)          { return a == b; }
std::string Executor::stringify(const FabValue& val)                     { return codefab::stringify(val); }

void Executor::checkNumber(const FabValue& val, const std::string&) {
    if (!std::holds_alternative<double>(val))
        throw RuntimeError("Operand must be a number.");
}
void Executor::checkNumbers(const FabValue& l, const FabValue& r, const std::string&) {
    if (!std::holds_alternative<double>(l) || !std::holds_alternative<double>(r))
        throw RuntimeError("Operands must be numbers.");
}

// ---- 구문 실행 (다음 커밋에서 구현) -----------------------------------------
void Executor::visitExpression(const ExpressionStmt&) {}
void Executor::visitPrint(const PrintStmt&)           {}
void Executor::visitVarDeclare(const VarDeclareStmt&) {}
void Executor::visitBlock(const BlockStmt&)           {}
void Executor::visitIf(const IfStmt&)                 {}
void Executor::visitFor(const ForStmt&)               {}

// ---- 표현식 평가 -------------------------------------------------------------

FabValue Executor::visitLiteral(const LiteralExpr& expr) { return expr.value; }

FabValue Executor::visitVariable(const VariableExpr& expr) {
    try { return environment->get(expr.name.origin); }
    catch (const RuntimeError&) {
        throw RuntimeError("[line " + std::to_string(expr.name.line) +
                           "] Undefined variable '" + expr.name.origin + "'.");
    }
}

FabValue Executor::visitAssign(const AssignExpr& expr) {
    FabValue val = evaluate(*expr.value);
    try { environment->assign(expr.name.origin, val); }
    catch (const RuntimeError&) {
        throw RuntimeError("[line " + std::to_string(expr.name.line) +
                           "] Undefined variable '" + expr.name.origin + "'.");
    }
    return val;
}

FabValue Executor::visitGrouping(const GroupingExpr& expr) { return evaluate(*expr.expression); }

FabValue Executor::visitUnary(const UnaryExpr& expr) {
    FabValue operand = evaluate(*expr.operand);
    switch (expr.op.type) {
        case TokenType::MINUS:
            checkNumber(operand, expr.op.origin);
            return -std::get<double>(operand);
        case TokenType::BANG:
            return !isTruthy(operand);
        default:
            throw RuntimeError("Unknown unary operator: " + expr.op.origin);
    }
}

FabValue Executor::visitBinary(const BinaryExpr& expr) {
    FabValue left  = evaluate(*expr.left);
    FabValue right = evaluate(*expr.right);
    switch (expr.op.type) {
        case TokenType::PLUS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) + std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) + std::get<std::string>(right);
            throw RuntimeError("Operands must be two numbers or two strings.");
        case TokenType::MINUS:
            checkNumbers(left, right, expr.op.origin);
            return std::get<double>(left) - std::get<double>(right);
        case TokenType::STAR:
            checkNumbers(left, right, expr.op.origin);
            return std::get<double>(left) * std::get<double>(right);
        case TokenType::SLASH:
            checkNumbers(left, right, expr.op.origin);
            if (std::get<double>(right) == 0.0) throw RuntimeError("Division by zero.");
            return std::get<double>(left) / std::get<double>(right);
        case TokenType::PERCENT:
            checkNumbers(left, right, expr.op.origin);
            if (std::get<double>(right) == 0.0) throw RuntimeError("Division by zero (modulo).");
            return std::fmod(std::get<double>(left), std::get<double>(right));
        case TokenType::GREATER:       checkNumbers(left, right, expr.op.origin); return std::get<double>(left) >  std::get<double>(right);
        case TokenType::GREATER_EQUAL: checkNumbers(left, right, expr.op.origin); return std::get<double>(left) >= std::get<double>(right);
        case TokenType::LESS:          checkNumbers(left, right, expr.op.origin); return std::get<double>(left) <  std::get<double>(right);
        case TokenType::LESS_EQUAL:    checkNumbers(left, right, expr.op.origin); return std::get<double>(left) <= std::get<double>(right);
        case TokenType::EQUAL_EQUAL:   return isEqualVal(left, right);
        case TokenType::BANG_EQUAL:    return !isEqualVal(left, right);
        default: throw RuntimeError("Unknown binary operator: " + expr.op.origin);
    }
}

FabValue Executor::visitLogical(const LogicalExpr& expr) {
    FabValue left = evaluate(*expr.left);
    if (expr.op.type == TokenType::OR)  { if (isTruthy(left))  return left; }
    else                                { if (!isTruthy(left)) return left; }
    return evaluate(*expr.right);
}

} // namespace codefab
