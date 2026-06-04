#include "codefab/assembler/lexer.h"
// [refactoring] 코드 정리 및 가독성 개선

namespace codefab {

// 예약어 목록: 식별자와 동일한 형태이므로 scanIdentifier에서 분류
const std::unordered_map<std::string, TokenType> Lexer::KEYWORDS = {
    {"var",   TokenType::VAR},   {"print", TokenType::PRINT},
    {"if",    TokenType::IF},    {"else",  TokenType::ELSE},
    {"for",   TokenType::FOR},   {"true",  TokenType::TRUE_TOKEN},
    {"false", TokenType::FALSE_TOKEN}, {"null", TokenType::NULL_TOKEN},
};

Lexer::Lexer(std::string src) : source(std::move(src)) {
    // UTF-8 BOM(EF BB BF) 자동 제거 — Windows 에디터가 삽입하는 BOM 방지
    if (source.size() >= 3 &&
        (unsigned char)source[0] == 0xEF &&
        (unsigned char)source[1] == 0xBB &&
        (unsigned char)source[2] == 0xBF) { source.erase(0, 3); }
}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) { start = current; scanToken(); }
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
        case '/':
            if (match('/')) { while (peek() != '\n' && !isAtEnd()) advance(); }
            else addToken(TokenType::SLASH);
            break;
        // 두 글자 연산자: match()로 다음 문자 확인
        case '!': addToken(match('=') ? TokenType::BANG_EQUAL  : TokenType::BANG);      break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);     break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL  : TokenType::LESS);      break;
        case '&': // Fab은 '&&'만 지원; 단독 '&'는 오류
            if (!match('&')) throw AssemblerError("'&&' 연산자를 사용하세요 (위치: " + std::to_string(current) + ")");
            addToken(TokenType::AND); break;
        case '|': // Fab은 '||'만 지원; 단독 '|'는 오류
            if (!match('|')) throw AssemblerError("'||' 연산자를 사용하세요 (위치: " + std::to_string(current) + ")");
            break;
        case ';': addToken(TokenType::SEMICOLON);   break;
        case '{': addToken(TokenType::LEFT_BRACE);  break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case '(': addToken(TokenType::LEFT_PAREN);  break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case ' ': case '\r': case '\t': break;
        case '\n': line++; break;
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
    addToken(TokenType::STRING, source.substr(start + 1, current - start - 2));
}

// 숫자: 정수 부분 → (선택) 소수점 + 소수 부분
void Lexer::scanNumber() {
    while (isDigit(peek())) advance();
    if (peek() == '.' && isDigit(peekNext())) { advance(); while (isDigit(peek())) advance(); }
    addToken(TokenType::NUMBER, source.substr(start, current - start));
}

// 식별자: 알파벳·숫자·밑줄 연속 → 키워드이면 해당 타입, 아니면 IDENTIFIER
void Lexer::scanIdentifier() {
    while (isAlphaNumeric(peek())) advance();
    std::string text = source.substr(start, current - start);
    auto it = KEYWORDS.find(text);
    addToken((it != KEYWORDS.end()) ? it->second : TokenType::IDENTIFIER);
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
