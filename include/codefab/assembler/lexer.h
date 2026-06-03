#pragma once
#include "../token.h"
#include "../errors.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace codefab {

// Assembler Unit - 1단계: 소스 코드 → Token 목록
class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string source;
    std::vector<Token> tokens;
    int start   = 0;
    int current = 0;
    int line    = 1;

    static const std::unordered_map<std::string, TokenType> KEYWORDS;

    void scanToken();
    void scanString();
    void scanNumber();
    void scanIdentifier();

    void addToken(TokenType type);
    void addToken(TokenType type, const std::string& lexeme);

    char advance();
    bool match(char expected);
    char peek() const;
    char peekNext() const;
    bool isAtEnd() const;

    static bool isDigit(char c);
    static bool isAlpha(char c);
    static bool isAlphaNumeric(char c);
};

} // namespace codefab
