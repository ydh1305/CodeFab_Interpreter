#pragma once
#include "../fab_value.h"
#include "../errors.h"
#include <unordered_map>
#include <memory>
#include <string>

namespace codefab {

// 변수 저장소: 이름 → 값 매핑
// parent 체인으로 스코프 계층 구조를 표현
class Environment {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr);

    // 현재 스코프에 변수 정의
    void define(const std::string& name, FabValue value);

    // 가장 가까운 스코프부터 탐색하여 변수 값을 반환
    FabValue get(const std::string& name) const;

    // 가장 가까운 스코프부터 탐색하여 변수 값을 갱신
    void assign(const std::string& name, FabValue value);

private:
    std::unordered_map<std::string, FabValue> values;
    std::shared_ptr<Environment> parent;
};

} // namespace codefab
