#include "codefab/assembler/parser.h"

namespace codefab {

Parser::Parser(std::vector<Token> toks) : tokens(std::move(toks)) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) statements.push_back(expressionStatement());
    return statements;
}

// 구문 파싱은 다음 커밋에서 구현
std::unique_ptr<Stmt> Parser::statement()      { return expressionStatement(); }
std::unique_ptr<Stmt> Parser::varDeclaration() { throw AssemblerError("미구현: var"); }
std::unique_ptr<Stmt> Parser::printStatement() { throw AssemblerError("미구현: print"); }
std::unique_ptr<Stmt> Parser::ifStatement()    { throw AssemblerError("미구현: if"); }
std::unique_ptr<Stmt> Parser::forStatement()   { throw AssemblerError("미구현: for"); }
std::vector<std::unique_ptr<Stmt>> Parser::block() { return {}; }

std::unique_ptr<Stmt> Parser::expressionStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

// assignment < logicalOr < logicalAnd < equality < comparison < term < factor < unary < primary
std::unique_ptr<Expr> Parser::expression() { return assignment(); }

std::unique_ptr<Expr> Parser::assignment() {
    auto expr = logicalOr();
    if (matchAny({TokenType::EQUAL})) {
        auto value = assignment();
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get()))
            return std::make_unique<AssignExpr>(var->name, std::move(value));
        throw AssemblerError("Invalid assignment target.");
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logicalOr() {
    auto expr = logicalAnd();
    while (matchAny({TokenType::OR})) {
        Token op = previous();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, logicalAnd());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logicalAnd() {
    auto expr = equality();
    while (matchAny({TokenType::AND})) {
        Token op = previous();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, equality());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();
    while (matchAny({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
        Token op = previous(); expr = std::make_unique<BinaryExpr>(std::move(expr), op, comparison());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term();
    while (matchAny({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous(); expr = std::make_unique<BinaryExpr>(std::move(expr), op, term());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();
    while (matchAny({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous(); expr = std::make_unique<BinaryExpr>(std::move(expr), op, factor());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    auto expr = unary();
    while (matchAny({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        Token op = previous(); expr = std::make_unique<BinaryExpr>(std::move(expr), op, unary());
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (matchAny({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        return std::make_unique<UnaryExpr>(op, unary());
    }
    return primary();
}

std::unique_ptr<Expr> Parser::primary() {
    if (isAtEnd()) throw IncompleteInputError("Unexpected end of input.");
    if (matchAny({TokenType::NUMBER})) return std::make_unique<LiteralExpr>(FabValue{std::stod(previous().origin)});
    if (matchAny({TokenType::STRING})) return std::make_unique<LiteralExpr>(FabValue{previous().origin});
    if (matchAny({TokenType::TRUE_TOKEN}))  return std::make_unique<LiteralExpr>(FabValue{true});
    if (matchAny({TokenType::FALSE_TOKEN})) return std::make_unique<LiteralExpr>(FabValue{false});
    if (matchAny({TokenType::NULL_TOKEN}))  return std::make_unique<LiteralExpr>(FabValue{nullptr});
    if (matchAny({TokenType::IDENTIFIER}))  return std::make_unique<VariableExpr>(previous());
    if (matchAny({TokenType::LEFT_PAREN})) {
        auto expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }
    throw AssemblerError("Expect expression.");
}

bool Parser::matchAny(std::initializer_list<TokenType> types) {
    for (auto t : types) { if (check(t)) { advance(); return true; } }
    return false;
}
bool Parser::check(TokenType t) const { return !isAtEnd() && tokens[current].type == t; }
Token& Parser::advance() { if (!isAtEnd()) current++; return previous(); }
Token& Parser::consume(TokenType t, const std::string& msg) {
    if (check(t)) return advance();
    if (isAtEnd()) throw IncompleteInputError(msg);
    throw AssemblerError(msg);
}
bool Parser::isAtEnd() const       { return tokens[current].type == TokenType::EOF_TOKEN; }
Token& Parser::peek()              { return tokens[current]; }
const Token& Parser::peek() const  { return tokens[current]; }
Token& Parser::previous()          { return tokens[current - 1]; }

} // namespace codefab
