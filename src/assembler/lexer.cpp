#include "codefab/assembler/lexer.h"

namespace codefab {

// G1: 키워드 미구분 (숫자·문자열 리터럴 우선 구현)
const std::unordered_map<std::string, TokenType> Lexer::KEYWORDS = {};

Lexer::Lexer(std::string src) : source(std::move(src)) {}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) { start = current; scanToken(); }
    tokens.emplace_back(TokenType::EOF_TOKEN, "");
    return std::move(tokens);
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case ' ': case '\r': case '\t': break;
        case '\n': line++; break;
        case '"': scanString(); break;
        default:
            if (isDigit(c))      scanNumber();
            else if (isAlpha(c)) scanIdentifier();
            else throw AssemblerError(std::string("예상치 못한 문자 '") + c + "'");
    }
}

void Lexer::scanString() {
    while (peek() != '"' && !isAtEnd()) advance();
    if (isAtEnd()) throw AssemblerError("종료되지 않은 문자열 리터럴");
    advance();
    addToken(TokenType::STRING, source.substr(start + 1, current - start - 2));
}

void Lexer::scanNumber() {
    while (isDigit(peek())) advance();
    if (peek() == '.' && isDigit(peekNext())) { advance(); while (isDigit(peek())) advance(); }
    addToken(TokenType::NUMBER, source.substr(start, current - start));
}

void Lexer::scanIdentifier() {
    while (isAlphaNumeric(peek())) advance();
    addToken(TokenType::IDENTIFIER); // 키워드 미구분: 모두 IDENTIFIER
}

void Lexer::addToken(TokenType t) { addToken(t, source.substr(start, current - start)); }
void Lexer::addToken(TokenType t, const std::string& l) { tokens.emplace_back(t, l, line); }
char Lexer::advance()  { return source[current++]; }
bool Lexer::match(char e) { if (isAtEnd() || source[current] != e) return false; current++; return true; }
char Lexer::peek()     const { return isAtEnd() ? '\0' : source[current]; }
char Lexer::peekNext() const { return (current+1 >= (int)source.size()) ? '\0' : source[current+1]; }
bool Lexer::isAtEnd()  const { return current >= (int)source.size(); }
bool Lexer::isDigit(char c)  { return c >= '0' && c <= '9'; }
bool Lexer::isAlpha(char c)  { return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'; }
bool Lexer::isAlphaNumeric(char c) { return isAlpha(c)||isDigit(c); }

} // namespace codefab
