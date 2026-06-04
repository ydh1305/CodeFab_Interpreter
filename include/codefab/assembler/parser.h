#pragma once
#include "../token.h"
#include "../ast/stmt.h"
#include "../errors.h"
#include <vector>
#include <memory>
#include <initializer_list>
namespace codefab {
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::vector<std::unique_ptr<Stmt>> parse();
private:
    std::vector<Token> tokens;
    int current = 0;
    std::unique_ptr<Stmt> statement(); std::unique_ptr<Stmt> varDeclaration();
    std::unique_ptr<Stmt> printStatement(); std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> forStatement(); std::vector<std::unique_ptr<Stmt>> block();
    std::unique_ptr<Stmt> expressionStatement();
    std::unique_ptr<Expr> expression(); std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> logicalOr(); std::unique_ptr<Expr> logicalAnd();
    std::unique_ptr<Expr> equality(); std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term(); std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary(); std::unique_ptr<Expr> primary();
    bool matchAny(std::initializer_list<TokenType> types);
    bool check(TokenType t) const; Token& advance();
    Token& consume(TokenType t, const std::string& msg);
    bool isAtEnd() const; Token& peek(); const Token& peek() const; Token& previous();
};
} // namespace codefab
