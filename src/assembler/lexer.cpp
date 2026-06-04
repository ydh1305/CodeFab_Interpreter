#include "codefab/assembler/lexer.h"

namespace codefab {

// 예약어 목록: 식별자와 동일한 형태이므로 scanIdentifier에서 분류
const std::unordered_map<std::string, TokenType> Lexer::KEYWORDS = {
    {"var",   TokenType::VAR},   {"print", TokenType::PRINT},
    {"if",    TokenType::IF},    {"else",  TokenType::ELSE},
    {"for",   TokenType::FOR},   {"true",  TokenType::TRUE_TOKEN},
    {"false", TokenType::FALSE_TOKEN}, {"null", TokenType::NULL_TOKEN},
};

Lexer::Lexer(std::string src) : m_source(std::move(src)) {
    // UTF-8 BOM(EF BB BF) 자동 제거 — Windows 에디터가 삽입하는 BOM 방지
    if (m_source.size() >= 3 &&
        (unsigned char)m_source[0] == 0xEF &&
        (unsigned char)m_source[1] == 0xBB &&
        (unsigned char)m_source[2] == 0xBF) { m_source.erase(0, 3); }
}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) { m_start = m_current; scanToken(); }
    // 항상 EOF_TOKEN으로 마무리하여 Parser가 명확한 종료 조건을 가짐
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
        case '/':
            if (match('/')) { while (peek() != '\n' && !isAtEnd()) advance(); }
            else addToken(TokenType::SLASH);
            break;
        // 두 글자 연산자: match()로 다음 문자 확인
        case '!': addToken(match('=') ? TokenType::BANG_EQUAL  : TokenType::BANG);      break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);     break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL  : TokenType::LESS);      break;
        case '&':
            if (match('&')) addToken(TokenType::AND);
            else throw AssemblerError(std::string("예상치 못한 문자 '&' (위치: ") + std::to_string(m_current) + ")");
            break;
        case '|':
            if (match('|')) addToken(TokenType::OR);
            else throw AssemblerError(std::string("예상치 못한 문자 '|' (위치: ") + std::to_string(m_current) + ")");
            break;
        case ';': addToken(TokenType::SEMICOLON);   break;
        case '{': addToken(TokenType::LEFT_BRACE);  break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case '(': addToken(TokenType::LEFT_PAREN);  break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case ' ': case '\r': case '\t': break;
        case '\n': m_line++; break;
        case '"': scanString(); break;
        default:
            if (isDigit(c))      scanNumber();
            else if (isAlpha(c)) scanIdentifier();
            else throw AssemblerError(std::string("예상치 못한 문자 '") + c + "'");
    }
}

// 문자열: 시작 따옴표(") ~ 닫는 따옴표 사이 내용을 origin으로 저장
void Lexer::scanString() {
    while (peek() != '"' && !isAtEnd()) advance();
    if (isAtEnd()) throw AssemblerError("종료되지 않은 문자열 리터럴");
    advance(); // 닫는 따옴표 소비
    addToken(TokenType::STRING, m_source.substr(m_start + 1, m_current - m_start - 2));
}

// 숫자: 정수 부분 → (선택) 소수점 + 소수 부분
void Lexer::scanNumber() {
    while (isDigit(peek())) advance();
    if (peek() == '.' && isDigit(peekNext())) { advance(); while (isDigit(peek())) advance(); }
    addToken(TokenType::NUMBER, m_source.substr(m_start, m_current - m_start));
}

// 식별자: 알파벳·숫자·밑줄 연속 → 키워드이면 해당 타입, 아니면 IDENTIFIER
void Lexer::scanIdentifier() {
    while (isAlphaNumeric(peek())) advance();
    std::string text = m_source.substr(m_start, m_current - m_start);
    auto it = KEYWORDS.find(text);
    addToken((it != KEYWORDS.end()) ? it->second : TokenType::IDENTIFIER);
}

void Lexer::addToken(TokenType t) { addToken(t, m_source.substr(m_start, m_current - m_start)); }
void Lexer::addToken(TokenType t, const std::string& l) { m_tokens.emplace_back(t, l, m_line); }
char Lexer::advance()  { return m_source[m_current++]; }
bool Lexer::match(char e) { if (isAtEnd() || m_source[m_current] != e) return false; m_current++; return true; }
char Lexer::peek()     const { return isAtEnd() ? '\0' : m_source[m_current]; }
char Lexer::peekNext() const { return (m_current+1 >= (int)m_source.size()) ? '\0' : m_source[m_current+1]; }
bool Lexer::isAtEnd()  const { return m_current >= (int)m_source.size(); }
bool Lexer::isDigit(char c)  { return c >= '0' && c <= '9'; }
bool Lexer::isAlpha(char c)  { return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'; }
bool Lexer::isAlphaNumeric(char c) { return isAlpha(c)||isDigit(c); }

} // namespace codefab
