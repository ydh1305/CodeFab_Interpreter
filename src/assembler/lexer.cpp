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

Lexer::Lexer(std::string source) : m_source(std::move(source)) {
    // UTF-8 BOM (EF BB BF) 자동 제거
    if (this->m_source.size() >= 3 &&
        static_cast<unsigned char>(this->m_source[0]) == 0xEF &&
        static_cast<unsigned char>(this->m_source[1]) == 0xBB &&
        static_cast<unsigned char>(this->m_source[2]) == 0xBF) {
        this->m_source.erase(0, 3);
    }
}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
        m_start = m_current;
        scanToken();
    }
    m_tokens.emplace_back(TokenType::EOF_TOKEN, "");
    return std::move(m_tokens);
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
                // 한 줄 주석: 줄 끝까지 건너뜀
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::SLASH);
            }
            break;

        case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);          break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);        break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);    break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);          break;

        case '&':
            if (match('&')) addToken(TokenType::AND);
            else throw AssemblerError(std::string("예상치 못한 문자 '&' (위치: ") + std::to_string(m_current) + ")");
            break;
        case '|':
            if (match('|')) addToken(TokenType::OR);
            else throw AssemblerError(std::string("예상치 못한 문자 '|' (위치: ") + std::to_string(m_current) + ")");
            break;

        case ' ':
        case '\r':
        case '\t':
            break; // 공백 무시
        case '\n':
            m_line++; // 줄 번호 증가
            break;

        case '"': scanString(); break;

        default:
            if (isDigit(c))       scanNumber();
            else if (isAlpha(c))  scanIdentifier();
            else throw AssemblerError(
                    std::string("예상치 못한 문자 '") + c +
                    "' (위치: " + std::to_string(m_current) + ")");
    }
}

void Lexer::scanString() {
    while (peek() != '"' && !isAtEnd()) advance();
    if (isAtEnd()) throw AssemblerError("종료되지 않은 문자열 리터럴");
    advance(); // 닫는 따옴표 소비
    // origin에는 따옴표를 제외한 문자열 내용만 저장
    std::string value = m_source.substr(m_start + 1, m_current - m_start - 2);
    addToken(TokenType::STRING, value);
}

void Lexer::scanNumber() {
    while (isDigit(peek())) advance();
    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // '.' 소비
        while (isDigit(peek())) advance();
    }
    addToken(TokenType::NUMBER, m_source.substr(m_start, m_current - m_start));
}

void Lexer::scanIdentifier() {
    while (isAlphaNumeric(peek())) advance();
    std::string text = m_source.substr(m_start, m_current - m_start);
    auto it = KEYWORDS.find(text);
    TokenType type = (it != KEYWORDS.end()) ? it->second : TokenType::IDENTIFIER;
    addToken(type);
}

void Lexer::addToken(TokenType type) {
    addToken(type, m_source.substr(m_start, m_current - m_start));
}

void Lexer::addToken(TokenType type, const std::string& lexeme) {
    m_tokens.emplace_back(type, lexeme, m_line);
}

char Lexer::advance() {
    return m_source[m_current++];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || m_source[m_current] != expected) return false;
    m_current++;
    return true;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return m_source[m_current];
}

char Lexer::peekNext() const {
    if (m_current + 1 >= static_cast<int>(m_source.size())) return '\0';
    return m_source[m_current + 1];
}

bool Lexer::isAtEnd() const {
    return m_current >= static_cast<int>(m_source.size());
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

} // namespace codefab
