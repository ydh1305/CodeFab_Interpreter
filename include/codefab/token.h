#pragma once
#include <string>

namespace codefab {

enum class TokenType {
    // 리터럴
    NUMBER, STRING, TRUE_TOKEN, FALSE_TOKEN, NULL_TOKEN,

    // 식별자
    IDENTIFIER,

    // 키워드
    VAR, PRINT, IF, ELSE, FOR,

    // 단일 문자 연산자
    PLUS, MINUS, STAR, SLASH, PERCENT,

    // 단일/이중 문자 연산자
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // 논리 연산자
    AND, OR,

    // 구분자
    SEMICOLON,
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_PAREN, RIGHT_PAREN,

    // 파일 끝
    EOF_TOKEN
};

// Token 클래스: type, origin(원본 문자열), line(줄 번호) 필드를 가진다
struct Token {
    TokenType type;
    std::string origin;
    int line;

    Token(TokenType type, std::string origin, int line = 0)
        : type(type), origin(std::move(origin)), line(line) {}
};

} // namespace codefab
