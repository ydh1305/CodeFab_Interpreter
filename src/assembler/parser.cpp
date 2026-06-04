#include "codefab/assembler/parser.h"
namespace codefab {
Parser::Parser(std::vector<Token> toks) : tokens(std::move(toks)) {}
std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!isAtEnd()) stmts.push_back(statement());
    return stmts;
}
std::unique_ptr<Stmt> Parser::statement() {
    if (matchAny({TokenType::VAR}))   return varDeclaration();
    if (matchAny({TokenType::PRINT})) return printStatement();
    return expressionStatement();
}
std::unique_ptr<Stmt> Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    std::unique_ptr<Expr> init = nullptr;
    if (matchAny({TokenType::EQUAL})) init = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<VarDeclareStmt>(std::move(name), std::move(init));
}
std::unique_ptr<Stmt> Parser::printStatement() {
    auto v = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_unique<PrintStmt>(std::move(v));
}
std::unique_ptr<Stmt> Parser::ifStatement()  { throw AssemblerError("if: not impl"); }
std::unique_ptr<Stmt> Parser::forStatement() { throw AssemblerError("for: not impl"); }
std::vector<std::unique_ptr<Stmt>> Parser::block() { return {}; }
std::unique_ptr<Stmt> Parser::expressionStatement() {
    auto e = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(e));
}
std::unique_ptr<Expr> Parser::expression()  { return assignment(); }
std::unique_ptr<Expr> Parser::assignment()  {
    auto e = logicalOr();
    if (matchAny({TokenType::EQUAL})) {
        auto v = assignment();
        if (auto* var = dynamic_cast<VariableExpr*>(e.get()))
            return std::make_unique<AssignExpr>(var->name, std::move(v));
        throw AssemblerError("Invalid assignment target.");
    }
    return e;
}
std::unique_ptr<Expr> Parser::logicalOr()   { auto e=logicalAnd(); while(matchAny({TokenType::OR})){Token op=previous();e=std::make_unique<LogicalExpr>(std::move(e),op,logicalAnd());}return e; }
std::unique_ptr<Expr> Parser::logicalAnd()  { auto e=equality();   while(matchAny({TokenType::AND})){Token op=previous();e=std::make_unique<LogicalExpr>(std::move(e),op,equality());}return e; }
std::unique_ptr<Expr> Parser::equality()    { auto e=comparison(); while(matchAny({TokenType::EQUAL_EQUAL,TokenType::BANG_EQUAL})){Token op=previous();e=std::make_unique<BinaryExpr>(std::move(e),op,comparison());}return e; }
std::unique_ptr<Expr> Parser::comparison()  { auto e=term();       while(matchAny({TokenType::GREATER,TokenType::GREATER_EQUAL,TokenType::LESS,TokenType::LESS_EQUAL})){Token op=previous();e=std::make_unique<BinaryExpr>(std::move(e),op,term());}return e; }
std::unique_ptr<Expr> Parser::term()        { auto e=factor();     while(matchAny({TokenType::PLUS,TokenType::MINUS})){Token op=previous();e=std::make_unique<BinaryExpr>(std::move(e),op,factor());}return e; }
std::unique_ptr<Expr> Parser::factor()      { auto e=unary();      while(matchAny({TokenType::STAR,TokenType::SLASH,TokenType::PERCENT})){Token op=previous();e=std::make_unique<BinaryExpr>(std::move(e),op,unary());}return e; }
std::unique_ptr<Expr> Parser::unary()       { if(matchAny({TokenType::BANG,TokenType::MINUS})){Token op=previous();return std::make_unique<UnaryExpr>(op,unary());}return primary(); }
std::unique_ptr<Expr> Parser::primary() {
    if (isAtEnd()) throw IncompleteInputError("Unexpected end of input.");
    if (matchAny({TokenType::NUMBER})) return std::make_unique<LiteralExpr>(FabValue{std::stod(previous().origin)});
    if (matchAny({TokenType::STRING})) return std::make_unique<LiteralExpr>(FabValue{previous().origin});
    if (matchAny({TokenType::TRUE_TOKEN}))  return std::make_unique<LiteralExpr>(FabValue{true});
    if (matchAny({TokenType::FALSE_TOKEN})) return std::make_unique<LiteralExpr>(FabValue{false});
    if (matchAny({TokenType::NULL_TOKEN}))  return std::make_unique<LiteralExpr>(FabValue{nullptr});
    if (matchAny({TokenType::IDENTIFIER}))  return std::make_unique<VariableExpr>(previous());
    if (matchAny({TokenType::LEFT_PAREN}))  { auto e=expression(); consume(TokenType::RIGHT_PAREN,"Expect ')' after expression."); return std::make_unique<GroupingExpr>(std::move(e)); }
    throw AssemblerError("Expect expression.");
}
bool Parser::matchAny(std::initializer_list<TokenType> ts) { for(auto t:ts){if(check(t)){advance();return true;}}return false; }
bool Parser::check(TokenType t) const { return !isAtEnd() && tokens[current].type == t; }
Token& Parser::advance()  { if(!isAtEnd()) current++; return previous(); }
Token& Parser::consume(TokenType t, const std::string& msg) { if(check(t)) return advance(); if(isAtEnd()) throw IncompleteInputError(msg); throw AssemblerError(msg); }
bool Parser::isAtEnd() const      { return tokens[current].type == TokenType::EOF_TOKEN; }
Token& Parser::peek()             { return tokens[current]; }
const Token& Parser::peek() const { return tokens[current]; }
Token& Parser::previous()         { return tokens[current-1]; }
} // namespace codefab
