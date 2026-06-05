#include "codefab/executor/executor.h"
#include "codefab/fab_value.h"
#include <iostream>
#include <cmath>

namespace codefab {

Executor::Executor()
    : m_environment(std::make_shared<Environment>()) {}

// ---- Public interface --------------------------------------------------------

void Executor::execute(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) {
        executeStmt(*stmt);
    }
}

// ---- Internal helpers --------------------------------------------------------

FabValue Executor::evaluate(const Expr& expr) {
    return expr.accept(*this);
}

void Executor::executeStmt(const Stmt& stmt) {
    stmt.accept(*this);
}

void Executor::executeBlock(const std::vector<std::unique_ptr<Stmt>>& stmts,
                            std::shared_ptr<Environment> env) {
    auto previous = m_environment;
    m_environment = std::move(env);
    try {
        for (const auto& stmt : stmts) {
            executeStmt(*stmt);
        }
    } catch (...) {
        m_environment = previous;
        throw;
    }
    m_environment = previous;
}

bool Executor::isTruthy(const FabValue& val) {
    return codefab::isTruthy(val);
}

bool Executor::isEqualVal(const FabValue& a, const FabValue& b) {
    return a == b;
}

void Executor::checkNumber(const FabValue& val, const std::string& /*op*/) {
    if (!std::holds_alternative<double>(val)) {
        throw RuntimeError("Operand must be a number.");
    }
}

void Executor::checkNumbers(const FabValue& left, const FabValue& right,
                            const std::string& /*op*/) {
    if (!std::holds_alternative<double>(left) || !std::holds_alternative<double>(right)) {
        throw RuntimeError("Operands must be numbers.");
    }
}

std::string Executor::stringify(const FabValue& val) {
    return codefab::stringify(val);
}

// ---- StmtVisitor -------------------------------------------------------------

void Executor::visitExpression(const ExpressionStmt& stmt) {
    evaluate(*stmt.expression);
}

void Executor::visitPrint(const PrintStmt& stmt) {
    FabValue val = evaluate(*stmt.expression);
    std::cout << stringify(val) << "\n";
}

void Executor::visitVarDeclare(const VarDeclareStmt& stmt) {
    FabValue val = nullptr;
    if (stmt.initializer) {
        val = evaluate(*stmt.initializer);
    }
    m_environment->define(stmt.name.origin, std::move(val));
}

void Executor::visitBlock(const BlockStmt& stmt) {
    executeBlock(stmt.statements, std::make_shared<Environment>(m_environment));
}

void Executor::visitIf(const IfStmt& stmt) {
    if (isTruthy(evaluate(*stmt.condition))) {
        executeStmt(*stmt.thenBranch);
    } else if (stmt.elseBranch) {
        executeStmt(*stmt.elseBranch);
    }
}

void Executor::visitFor(const ForStmt& stmt) {
    auto forEnv = std::make_shared<Environment>(m_environment);
    auto previous = m_environment;
    m_environment = forEnv;
    try {
        if (stmt.initializer) executeStmt(*stmt.initializer);

        while (!stmt.condition || isTruthy(evaluate(*stmt.condition))) {
            executeStmt(*stmt.body);
            if (stmt.increment) evaluate(*stmt.increment);
        }
    } catch (...) {
        m_environment = previous;
        throw;
    }
    m_environment = previous;
}

// ---- ExprVisitor -------------------------------------------------------------

FabValue Executor::visitLiteral(const LiteralExpr& expr) {
    return expr.value;
}

FabValue Executor::visitVariable(const VariableExpr& expr) {
    try {
        return m_environment->get(expr.name.origin);
    } catch (const RuntimeError&) {
        // 줄 번호 포함한 에러 메시지로 재throw
        throw RuntimeError(
            "[line " + std::to_string(expr.name.line) +
            "] Undefined variable '" + expr.name.origin + "'.");
    }
}

FabValue Executor::visitAssign(const AssignExpr& expr) {
    FabValue val = evaluate(*expr.value);
    try {
        m_environment->assign(expr.name.origin, val);
    } catch (const RuntimeError&) {
        throw RuntimeError(
            "[line " + std::to_string(expr.name.line) +
            "] Undefined variable '" + expr.name.origin + "'.");
    }
    return val;
}

FabValue Executor::visitGrouping(const GroupingExpr& expr) {
    return evaluate(*expr.expression);
}

FabValue Executor::visitUnary(const UnaryExpr& expr) {
    FabValue operand = evaluate(*expr.operand);
    switch (expr.op.type) {
        case TokenType::MINUS:
            checkNumber(operand, expr.op.origin);  // "Operand must be a number."
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
            // 숫자 + 숫자
            if (std::holds_alternative<double>(left) &&
                std::holds_alternative<double>(right)) {
                return std::get<double>(left) + std::get<double>(right);
            }
            // 문자열 + 문자열
            if (std::holds_alternative<std::string>(left) &&
                std::holds_alternative<std::string>(right)) {
                return std::get<std::string>(left) + std::get<std::string>(right);
            }
            // 혼용 불가
            throw RuntimeError("Operands must be two numbers or two strings.");

        case TokenType::MINUS:
            checkNumbers(left, right, expr.op.origin);
            return std::get<double>(left) - std::get<double>(right);

        case TokenType::STAR:
            checkNumbers(left, right, expr.op.origin);
            return std::get<double>(left) * std::get<double>(right);

        case TokenType::SLASH:
            checkNumbers(left, right, expr.op.origin);
            if (std::get<double>(right) == 0.0)
                throw RuntimeError("Division by zero.");
            return std::get<double>(left) / std::get<double>(right);

        case TokenType::PERCENT:
            checkNumbers(left, right, expr.op.origin);
            if (std::get<double>(right) == 0.0)
                throw RuntimeError("Division by zero (modulo).");
            return std::fmod(std::get<double>(left), std::get<double>(right));

        case TokenType::GREATER:
            checkNumbers(left, right, expr.op.origin);
            return std::get<double>(left) > std::get<double>(right);

        case TokenType::GREATER_EQUAL:
            checkNumbers(left, right, expr.op.origin);
            return std::get<double>(left) >= std::get<double>(right);

        case TokenType::LESS:
            checkNumbers(left, right, expr.op.origin);
            return std::get<double>(left) < std::get<double>(right);

        case TokenType::LESS_EQUAL:
            checkNumbers(left, right, expr.op.origin);
            return std::get<double>(left) <= std::get<double>(right);

        case TokenType::EQUAL_EQUAL:
            return isEqualVal(left, right);

        case TokenType::BANG_EQUAL:
            return !isEqualVal(left, right);

        default:
            throw RuntimeError("Unknown binary operator: " + expr.op.origin);
    }
}

FabValue Executor::visitLogical(const LogicalExpr& expr) {
    FabValue left = evaluate(*expr.left);
    // 단락 평가 (Short-circuit evaluation)
    if (expr.op.type == TokenType::OR) {
        if (isTruthy(left)) return left;
    } else { // AND
        if (!isTruthy(left)) return left;
    }
    return evaluate(*expr.right);
}

} // namespace codefab
