#pragma once
#include <variant>
#include <string>
#include <cmath>
#include <sstream>

namespace codefab {

// 런타임 값 타입: null, boolean, number(double), string
using FabValue = std::variant<std::nullptr_t, bool, double, std::string>;

inline bool isTruthy(const FabValue& val) {
    if (std::holds_alternative<std::nullptr_t>(val)) return false;
    if (std::holds_alternative<bool>(val)) return std::get<bool>(val);
    return true; // 숫자, 문자열은 truthy
}

inline bool isEqual(const FabValue& a, const FabValue& b) {
    return a == b; // std::variant 동등 비교 (타입 + 값 모두 비교)
}

inline std::string typeName(const FabValue& val) {
    if (std::holds_alternative<std::nullptr_t>(val)) return "null";
    if (std::holds_alternative<bool>(val)) return "boolean";
    if (std::holds_alternative<double>(val)) return "number";
    return "string";
}

inline std::string stringify(const FabValue& val) {
    if (std::holds_alternative<std::nullptr_t>(val)) return "null";
    if (std::holds_alternative<bool>(val)) return std::get<bool>(val) ? "true" : "false";
    if (std::holds_alternative<double>(val)) {
        double d = std::get<double>(val);
        // 정수인 경우 소수점 없이 출력
        if (d == std::floor(d) && !std::isinf(d)) {
            std::ostringstream oss;
            oss << static_cast<long long>(d);
            return oss.str();
        }
        std::ostringstream oss;
        oss << d;
        return oss.str();
    }
    return std::get<std::string>(val);
}

} // namespace codefab
