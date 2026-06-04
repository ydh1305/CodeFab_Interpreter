#pragma once
// ============================================================
// Mock Lexer Helper
// ------------------------------------------------------------
// Lexer가 아직 feature/lexer 브랜치에서 개발 중이므로,
// Parser 독립 개발을 위해 Token을 직접 생성하는 헬퍼를 사용.
// feature/lexer → tdd-commits 병합 후 real Lexer로 교체 예정.
// ============================================================

#include "codefab/token.h"
#include <vector>
#include <initializer_list>

namespace codefab::mock {

inline std::vector<Token> makeTokens(
    std::initializer_list<std::pair<TokenType, std::string>> specs)
{
    std::vector<Token> tokens;
    for (auto& [type, origin] : specs) {
        tokens.emplace_back(type, origin);
    }
    tokens.emplace_back(TokenType::EOF_TOKEN, "");
    return tokens;
}

} // namespace codefab::mock
