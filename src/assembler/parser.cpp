#include "codefab/assembler/parser.h"
#include <stdexcept>

namespace codefab {

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        statements.push_back(statement());
    }
    return statements;
}

// ---- Statement parsing -------------------------------------------------------

std::unique_ptr<Stmt> Parser::statement() {
    if (matchAny({TokenType::VAR}))        return varDeclaration();
    if (matchAny({TokenType::PRINT}))      return printStatement();
    if (matchAny({TokenType::IF}))         return ifStatement();
    if (matchAny({TokenType::FOR}))        return forStatement();
    if (matchAny({TokenType::LEFT_BRACE})) return std::make_unique<BlockStmt>(block());
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

    std::unique_ptr<Expr> initializer = nullptr;
    if (matchAny({TokenType::EQUAL})) {
        initializer = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<VarDeclareStmt>(std::move(name), std::move(initializer));
}

std::unique_ptr<Stmt> Parser::printStatement() {
    auto value = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_unique<PrintStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    auto cond = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

    auto thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (matchAny({TokenType::ELSE})) {
        elseBranch = statement();
    }
    return std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::forStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    std::unique_ptr<Stmt> initializer = nullptr;
    if (matchAny({TokenType::SEMICOLON})) {
        // initializer omitted
    } else if (matchAny({TokenType::VAR})) {
        initializer = varDeclaration();
    } else {
        initializer = expressionStatement();
    }

    std::unique_ptr<Expr> condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    std::unique_ptr<Expr> increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    auto body = statement();
    return std::make_unique<ForStmt>(
        std::move(initializer), std::move(condition),
        std::move(increment), std::move(body)
    );
}

std::vector<std::unique_ptr<Stmt>> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(statement());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return statements;
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

// ---- Expression parsing (lowest to highest precedence) ----------------------

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    auto expr = logicalOr();

    if (matchAny({TokenType::EQUAL})) {
        auto value = assignment();
        auto* var = dynamic_cast<VariableExpr*>(expr.get());
        if (!var) throw AssemblerError("Invalid assignment target.");
        Token name = var->name;
        return std::make_unique<AssignExpr>(std::move(name), std::move(value));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logicalOr() {
    auto expr = logicalAnd();
    while (matchAny({TokenType::OR})) {
        Token op = previous();
        auto right = logicalAnd();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logicalAnd() {
    auto expr = equality();
    while (matchAny({TokenType::AND})) {
        Token op = previous();
        auto right = equality();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();
    while (matchAny({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
        Token op = previous();
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term();
    while (matchAny({TokenType::GREATER, TokenType::GREATER_EQUAL,
                     TokenType::LESS,    TokenType::LESS_EQUAL})) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();
    while (matchAny({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    auto expr = unary();
    while (matchAny({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        Token op = previous();
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (matchAny({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        auto operand = unary();
        return std::make_unique<UnaryExpr>(std::move(op), std::move(operand));
    }
    return primary();
}

std::unique_ptr<Expr> Parser::primary() {
    if (isAtEnd()) throw IncompleteInputError("Unexpected end of input.");
    if (matchAny({TokenType::NUMBER})) {
        double val = std::stod(previous().origin);
        return std::make_unique<LiteralExpr>(FabValue{val});
    }
    if (matchAny({TokenType::STRING})) {
        return std::make_unique<LiteralExpr>(FabValue{previous().origin});
    }
    if (matchAny({TokenType::TRUE_TOKEN}))  return std::make_unique<LiteralExpr>(FabValue{true});
    if (matchAny({TokenType::FALSE_TOKEN})) return std::make_unique<LiteralExpr>(FabValue{false});
    if (matchAny({TokenType::NULL_TOKEN}))  return std::make_unique<LiteralExpr>(FabValue{nullptr});

    if (matchAny({TokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(previous());
    }
    if (matchAny({TokenType::LEFT_PAREN})) {
        auto expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }
    throw AssemblerError("Expect expression.");
}

// ---- Utilities ---------------------------------------------------------------

bool Parser::matchAny(std::initializer_list<TokenType> types) {
    for (auto type : types) {
        if (check(type)) { advance(); return true; }
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return m_tokens[m_current].type == type;
}

Token& Parser::advance() {
    if (!isAtEnd()) m_current++;
    return previous();
}

Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    // EOF에서 예상 토큰을 찾지 못하면 입력이 미완성 → REPL에서 계속 입력받도록
    if (isAtEnd()) throw IncompleteInputError(message);
    throw AssemblerError(message);
}

bool Parser::isAtEnd() const {
    return m_tokens[m_current].type == TokenType::EOF_TOKEN;
}

Token& Parser::peek() {
    return m_tokens[m_current];
}

const Token& Parser::peek() const {
    return m_tokens[m_current];
}

Token& Parser::previous() {
    return m_tokens[m_current - 1];
}

} // namespace codefab
