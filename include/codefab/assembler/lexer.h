#pragma once
#include "../token.h"
#include "../errors.h"
#include <vector>
#include <string>
#include <unordered_map>
namespace codefab {
class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();
private:
    std::string m_source;
    std::vector<Token> m_tokens;
    int start = 0, current = 0, line = 1;
    static const std::unordered_map<std::string, TokenType> KEYWORDS;
    void scanToken(); void scanString(); void scanNumber(); void scanIdentifier();
    void addToken(TokenType t); void addToken(TokenType t, const std::string& lexeme);
    char advance(); bool match(char e); char peek() const; char peekNext() const; bool isAtEnd() const;
    static bool isDigit(char c); static bool isAlpha(char c); static bool isAlphaNumeric(char c);
};
} // namespace codefab
