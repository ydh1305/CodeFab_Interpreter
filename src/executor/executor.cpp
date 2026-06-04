#include "codefab/executor/executor.h"
#include "codefab/fab_value.h"
#include <iostream>
#include <cmath>

namespace codefab {

Executor::Executor() : m_environment(std::make_shared<Environment>()) {}

void Executor::execute(const std::vector<std::unique_ptr<Stmt>>& stmts) {
    for (const auto& s : stmts) executeStmt(*s);
}

FabValue Executor::evaluate(const Expr& expr) { return expr.accept(*this); }
void Executor::executeStmt(const Stmt& stmt)  { stmt.accept(*this); }

void Executor::executeBlock(const std::vector<std::unique_ptr<Stmt>>& stmts,
                            std::shared_ptr<Environment> env) {
    auto prev = m_environment;
    m_environment = std::move(env);
    try { for (const auto& s : stmts) executeStmt(*s); }
    catch (...) { m_environment = prev; throw; }
    m_environment = prev;
}

bool Executor::isTruthy(const FabValue& v)                    { return codefab::isTruthy(v); }
bool Executor::isEqualVal(const FabValue& a, const FabValue& b){ return a == b; }
std::string Executor::stringify(const FabValue& v)            { return codefab::stringify(v); }

void Executor::checkNumber(const FabValue& v, const std::string&) {
    if (!std::holds_alternative<double>(v)) throw RuntimeError("Operand must be a number.");
}
void Executor::checkNumbers(const FabValue& l, const FabValue& r, const std::string&) {
    if (!std::holds_alternative<double>(l) || !std::holds_alternative<double>(r))
        throw RuntimeError("Operands must be numbers.");
}

// ---- 구문 실행 ---------------------------------------------------------------

void Executor::visitExpression(const ExpressionStmt& s) { evaluate(*s.expression); }

void Executor::visitPrint(const PrintStmt& s) {
    std::cout << stringify(evaluate(*s.expression)) << "\n";
}

void Executor::visitVarDeclare(const VarDeclareStmt& s) {
    FabValue val = nullptr;
    if (s.initializer) val = evaluate(*s.initializer);
    m_environment->define(s.name.origin, std::move(val));
}

void Executor::visitBlock(const BlockStmt& s) {
    executeBlock(s.statements, std::make_shared<Environment>(m_environment));
}

void Executor::visitIf(const IfStmt& s) {
    if (isTruthy(evaluate(*s.condition))) executeStmt(*s.thenBranch);
    else if (s.elseBranch)               executeStmt(*s.elseBranch);
}

void Executor::visitFor(const ForStmt& s) {
    auto forEnv = std::make_shared<Environment>(m_environment);
    auto prev = m_environment;
    m_environment = forEnv;
    try {
        if (s.initializer) executeStmt(*s.initializer);
        while (!s.condition || isTruthy(evaluate(*s.condition))) {
            executeStmt(*s.body);
            if (s.increment) evaluate(*s.increment);
        }
    } catch (...) { m_environment = prev; throw; }
    m_environment = prev;
}

// ---- 표현식 평가 -------------------------------------------------------------

FabValue Executor::visitLiteral(const LiteralExpr& e) { return e.value; }

FabValue Executor::visitVariable(const VariableExpr& e) {
    try { return m_environment->get(e.name.origin); }
    catch (const RuntimeError&) {
        throw RuntimeError("[line " + std::to_string(e.name.line) +
                           "] Undefined variable '" + e.name.origin + "'.");
    }
}

FabValue Executor::visitAssign(const AssignExpr& e) {
    FabValue val = evaluate(*e.value);
    try { m_environment->assign(e.name.origin, val); }
    catch (const RuntimeError&) {
        throw RuntimeError("[line " + std::to_string(e.name.line) +
                           "] Undefined variable '" + e.name.origin + "'.");
    }
    return val;
}

FabValue Executor::visitGrouping(const GroupingExpr& e) { return evaluate(*e.expression); }

FabValue Executor::visitUnary(const UnaryExpr& e) {
    FabValue op = evaluate(*e.operand);
    if (e.op.type == TokenType::MINUS) { checkNumber(op, e.op.origin); return -std::get<double>(op); }
    if (e.op.type == TokenType::BANG)  return !isTruthy(op);
    throw RuntimeError("Unknown unary operator.");
}

FabValue Executor::visitBinary(const BinaryExpr& e) {
    FabValue l = evaluate(*e.left), r = evaluate(*e.right);
    switch (e.op.type) {
        case TokenType::PLUS:
            if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r))
                return std::get<double>(l) + std::get<double>(r);
            if (std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r))
                return std::get<std::string>(l) + std::get<std::string>(r);
            throw RuntimeError("Operands must be two numbers or two strings.");
        case TokenType::MINUS:       checkNumbers(l,r,e.op.origin); return std::get<double>(l)-std::get<double>(r);
        case TokenType::STAR:        checkNumbers(l,r,e.op.origin); return std::get<double>(l)*std::get<double>(r);
        case TokenType::SLASH:       checkNumbers(l,r,e.op.origin);
            if (std::get<double>(r)==0.0) throw RuntimeError("Division by zero.");
            return std::get<double>(l)/std::get<double>(r);
        case TokenType::PERCENT:     checkNumbers(l,r,e.op.origin);
            if (std::get<double>(r)==0.0) throw RuntimeError("Division by zero (modulo).");
            return std::fmod(std::get<double>(l),std::get<double>(r));
        case TokenType::GREATER:       checkNumbers(l,r,e.op.origin); return std::get<double>(l)>std::get<double>(r);
        case TokenType::GREATER_EQUAL: checkNumbers(l,r,e.op.origin); return std::get<double>(l)>=std::get<double>(r);
        case TokenType::LESS:          checkNumbers(l,r,e.op.origin); return std::get<double>(l)<std::get<double>(r);
        case TokenType::LESS_EQUAL:    checkNumbers(l,r,e.op.origin); return std::get<double>(l)<=std::get<double>(r);
        case TokenType::EQUAL_EQUAL:   return isEqualVal(l, r);
        case TokenType::BANG_EQUAL:    return !isEqualVal(l, r);
        default: throw RuntimeError("Unknown binary operator.");
    }
}

FabValue Executor::visitLogical(const LogicalExpr& e) {
    FabValue l = evaluate(*e.left);
    if (e.op.type == TokenType::OR)  { if (isTruthy(l))  return l; }
    else                             { if (!isTruthy(l)) return l; }
    return evaluate(*e.right);
}

} // namespace codefab
