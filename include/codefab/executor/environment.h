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

    void define(const std::string& name, FabValue value);
    FabValue get(const std::string& name) const;
    void assign(const std::string& name, FabValue value);

private:
    std::unordered_map<std::string, FabValue> values;
    std::shared_ptr<Environment> parent;
};

} // namespace codefab
