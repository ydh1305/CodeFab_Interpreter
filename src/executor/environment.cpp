#include "codefab/executor/environment.h"

namespace codefab {

Environment::Environment(std::shared_ptr<Environment> parent)
    : parent(std::move(parent)) {}

void Environment::define(const std::string& name, FabValue value) {
    values[name] = std::move(value);
}

FabValue Environment::get(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end()) return it->second;
    if (parent) return parent->get(name);
    throw RuntimeError("정의되지 않은 변수 '" + name + "'");
}

void Environment::assign(const std::string& name, FabValue value) {
    auto it = values.find(name);
    if (it != values.end()) {
        it->second = std::move(value);
        return;
    }
    if (parent) {
        parent->assign(name, std::move(value));
        return;
    }
    throw RuntimeError("정의되지 않은 변수 '" + name + "'");
}

} // namespace codefab
