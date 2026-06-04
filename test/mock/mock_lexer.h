#pragma once
// ============================================================
// Mock Lexer Helper — Lexer 미병합 상태에서 Parser 독립 개발
// feature/lexer → tdd-commits 병합 후 실제 Lexer로 교체 예정
// ============================================================
#include "codefab/token.h"
#include <vector>
#include <initializer_list>
namespace codefab::mock {
inline std::vector<Token> makeTokens(
    std::initializer_list<std::pair<TokenType, std::string>> specs)
{
    std::vector<Token> tokens;
    for (auto& [type, origin] : specs) tokens.emplace_back(type, origin);
    tokens.emplace_back(TokenType::EOF_TOKEN, "");
    return tokens;
}
} // namespace codefab::mock
