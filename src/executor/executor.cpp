#include "codefab/executor/executor.h"
// [refactoring] 코드 정리 및 가독성 개선
#include "codefab/fab_value.h"
#include <iostream>
#include <cmath>
namespace codefab {
Executor::Executor() : environment(std::make_shared<Environment>()) {}
void Executor::execute(const std::vector<std::unique_ptr<Stmt>>& s) { for(const auto& st:s) executeStmt(*st); }
FabValue Executor::evaluate(const Expr& e)  { return e.accept(*this); }
void Executor::executeStmt(const Stmt& s)   { s.accept(*this); }
void Executor::executeBlock(const std::vector<std::unique_ptr<Stmt>>& stmts, std::shared_ptr<Environment> env) {
    auto prev = environment; environment = std::move(env);
    try { for(const auto& s:stmts) executeStmt(*s); } catch(...) { environment=prev; throw; }
    environment = prev;
}
bool Executor::isTruthy(const FabValue& v)                    { return codefab::isTruthy(v); }
bool Executor::isEqualVal(const FabValue& a, const FabValue& b){ return a==b; }
std::string Executor::stringify(const FabValue& v)            { return codefab::stringify(v); }
void Executor::checkNumber(const FabValue& v, const std::string&) { if(!std::holds_alternative<double>(v)) throw RuntimeError("Operand must be a number."); }
void Executor::checkNumbers(const FabValue& l, const FabValue& r, const std::string&) { if(!std::holds_alternative<double>(l)||!std::holds_alternative<double>(r)) throw RuntimeError("Operands must be numbers."); }
// 구문 실행 — 다음 커밋에서 구현
void Executor::visitExpression(const ExpressionStmt&) {}
// stringify(): FabValue → 출력 문자열 (정수는 소수점 없이)
void Executor::visitPrint(const PrintStmt& s) { std::cout << stringify(evaluate(*s.expression)) << "\n"; }
// 미초기화 변수는 null로 초기화
void Executor::visitVarDeclare(const VarDeclareStmt& s) { FabValue v=nullptr; if(s.initializer) v=evaluate(*s.initializer); environment->define(s.name.origin,std::move(v)); }
void Executor::visitBlock(const BlockStmt&)  {}
void Executor::visitIf(const IfStmt&)        {}
void Executor::visitFor(const ForStmt&)      {}
// 표현식 평가
FabValue Executor::visitLiteral(const LiteralExpr& e)   { return e.value; }
FabValue Executor::visitVariable(const VariableExpr& e) { try { return environment->get(e.name.origin); } catch(const RuntimeError&) { throw RuntimeError("[line "+std::to_string(e.name.line)+"] Undefined variable '"+e.name.origin+"'."); } }
FabValue Executor::visitAssign(const AssignExpr& e)     { FabValue v=evaluate(*e.value); try { environment->assign(e.name.origin,v); } catch(const RuntimeError&) { throw RuntimeError("[line "+std::to_string(e.name.line)+"] Undefined variable '"+e.name.origin+"'."); } return v; }
FabValue Executor::visitGrouping(const GroupingExpr& e) { return evaluate(*e.expression); }
FabValue Executor::visitUnary(const UnaryExpr& e)       { FabValue op=evaluate(*e.operand); if(e.op.type==TokenType::MINUS){checkNumber(op,e.op.origin);return -std::get<double>(op);}if(e.op.type==TokenType::BANG) return !isTruthy(op); throw RuntimeError("Unknown unary."); }
FabValue Executor::visitBinary(const BinaryExpr& e) {
    FabValue l=evaluate(*e.left), r=evaluate(*e.right);
    switch(e.op.type) {
        case TokenType::PLUS: {
            const bool bothNums = std::holds_alternative<double>(l) && std::holds_alternative<double>(r);
            const bool bothStrs = std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r);
            if (bothNums) return std::get<double>(l) + std::get<double>(r);
            if (bothStrs) return std::get<std::string>(l) + std::get<std::string>(r);
            throw RuntimeError("Operands must be two numbers or two strings.");
        }
        case TokenType::MINUS: checkNumbers(l,r,e.op.origin); return std::get<double>(l)-std::get<double>(r);
        case TokenType::STAR:  checkNumbers(l,r,e.op.origin); return std::get<double>(l)*std::get<double>(r);
        case TokenType::SLASH: checkNumbers(l,r,e.op.origin); if(std::get<double>(r)==0.0) throw RuntimeError("Division by zero."); return std::get<double>(l)/std::get<double>(r);
        case TokenType::PERCENT: checkNumbers(l,r,e.op.origin); if(std::get<double>(r)==0.0) throw RuntimeError("Division by zero (modulo)."); return std::fmod(std::get<double>(l),std::get<double>(r));
        case TokenType::GREATER:       checkNumbers(l,r,e.op.origin); return std::get<double>(l)>std::get<double>(r);
        case TokenType::GREATER_EQUAL: checkNumbers(l,r,e.op.origin); return std::get<double>(l)>=std::get<double>(r);
        case TokenType::LESS:          checkNumbers(l,r,e.op.origin); return std::get<double>(l)<std::get<double>(r);
        case TokenType::LESS_EQUAL:    checkNumbers(l,r,e.op.origin); return std::get<double>(l)<=std::get<double>(r);
        case TokenType::EQUAL_EQUAL: return isEqualVal(l,r);
        case TokenType::BANG_EQUAL:  return !isEqualVal(l,r);
        default: throw RuntimeError("Unknown binary operator.");
    }
}
FabValue Executor::visitLogical(const LogicalExpr& e) { FabValue l=evaluate(*e.left); if(e.op.type==TokenType::OR){if(isTruthy(l))return l;}else{if(!isTruthy(l))return l;} return evaluate(*e.right); }
} // namespace codefab
