#pragma once
#include <stdexcept>
#include <string>

namespace codefab {

struct AssemblerError : public std::runtime_error {
    explicit AssemblerError(const std::string& msg)
        : std::runtime_error(msg) {}
};

struct CheckerError : public std::runtime_error {
    explicit CheckerError(const std::string& msg)
        : std::runtime_error(msg) {}
};

struct RuntimeError : public std::runtime_error {
    explicit RuntimeError(const std::string& msg)
        : std::runtime_error(msg) {}
};

// REPL 전용: EOF에서 파싱이 중단됨 (입력이 아직 미완성)
// AssemblerError의 서브클래스 → EXPECT_THROW(..., AssemblerError) 테스트 호환 유지
// REPL의 tryParse()는 이 타입을 먼저 잡아 INCOMPLETE 상태로 전환
struct IncompleteInputError : public AssemblerError {
    explicit IncompleteInputError(const std::string& msg)
        : AssemblerError(msg) {}
};

} // namespace codefab
