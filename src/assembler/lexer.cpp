#include "codefab/assembler/lexer.h"
#include <stdexcept>

namespace codefab {

const std::unordered_map<std::string, TokenType> Lexer::KEYWORDS = {
    {"var",   TokenType::VAR},
    {"print", TokenType::PRINT},
    {"if",    TokenType::IF},
    {"else",  TokenType::ELSE},
    {"for",   TokenType::FOR},
    {"true",  TokenType::TRUE_TOKEN},
    {"false", TokenType::FALSE_TOKEN},
    {"null",  TokenType::NULL_TOKEN},
};

Lexer::Lexer(std::string src) : source(std::move(src)) {
    // UTF-8 BOM (EF BB BF) 자동 제거
    if (this->source.size() >= 3 &&
        static_cast<unsigned char>(this->source[0]) == 0xEF &&
        static_cast<unsigned char>(this->source[1]) == 0xBB &&
        static_cast<unsigned char>(this->source[2]) == 0xBF) {
        this->source.erase(0, 3);
    }
}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokens.emplace_back(TokenType::EOF_TOKEN, "");
    return std::move(tokens);
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '+': addToken(TokenType::PLUS);    break;
        case '-': addToken(TokenType::MINUS);   break;
        case '*': addToken(TokenType::STAR);    break;
        case '%': addToken(TokenType::PERCENT); break;
        case ';': addToken(TokenType::SEMICOLON);    break;
        case '{': addToken(TokenType::LEFT_BRACE);   break;
        case '}': addToken(TokenType::RIGHT_BRACE);  break;
        case '(': addToken(TokenType::LEFT_PAREN);   break;
        case ')': addToken(TokenType::RIGHT_PAREN);  break;

        case '/':
            if (match('/')) {
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::SLASH);
            }
            break;

        case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);        break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);      break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);  break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);        break;

        case '&':
            if (match('&')) addToken(TokenType::AND);
            else throw AssemblerError(std::string("예상치 못한 문자 '&' (위치: ") + std::to_string(current) + ")");
            break;
        case '|':
            if (match('|')) addToken(TokenType::OR);
            else throw AssemblerError(std::string("예상치 못한 문자 '|' (위치: ") + std::to_string(current) + ")");
            break;

        case ' ': case '\r': case '\t': break;
        case '\n': line++; break;
        case '"': scanString(); break;

        default:
            if (isDigit(c))      scanNumber();
            else if (isAlpha(c)) scanIdentifier();
            else throw AssemblerError(
                std::string("예상치 못한 문자 '") + c + "' (위치: " + std::to_string(current) + ")");
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
    if (peek() == '.' && isDigit(peekNext())) {
        advance();
        while (isDigit(peek())) advance();
    }
    addToken(TokenType::NUMBER, source.substr(start, current - start));
}

void Lexer::scanIdentifier() {
    while (isAlphaNumeric(peek())) advance();
    std::string text = source.substr(start, current - start);
    auto it = KEYWORDS.find(text);
    TokenType type = (it != KEYWORDS.end()) ? it->second : TokenType::IDENTIFIER;
    addToken(type);
}

void Lexer::addToken(TokenType type) {
    addToken(type, source.substr(start, current - start));
}

void Lexer::addToken(TokenType type, const std::string& lexeme) {
    tokens.emplace_back(type, lexeme, line);
}

char Lexer::advance()  { return source[current++]; }
bool Lexer::match(char expected) {
    if (isAtEnd() || source[current] != expected) return false;
    current++;
    return true;
}
char Lexer::peek() const     { return isAtEnd() ? '\0' : source[current]; }
char Lexer::peekNext() const {
    if (current + 1 >= static_cast<int>(source.size())) return '\0';
    return source[current + 1];
}
bool Lexer::isAtEnd() const  { return current >= static_cast<int>(source.size()); }
bool Lexer::isDigit(char c)  { return c >= '0' && c <= '9'; }
bool Lexer::isAlpha(char c)  { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool Lexer::isAlphaNumeric(char c) { return isAlpha(c) || isDigit(c); }

} // namespace codefab
