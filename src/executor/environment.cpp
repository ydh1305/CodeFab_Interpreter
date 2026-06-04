#include "codefab/executor/environment.h"
namespace codefab {
Environment::Environment(std::shared_ptr<Environment> p) : parent(std::move(p)) {}
void Environment::define(const std::string& name, FabValue v) { values[name] = std::move(v); }
FabValue Environment::get(const std::string& name) const {
    auto it = values.find(name);
    if (it != values.end()) return it->second;
    if (parent) return parent->get(name);
    throw RuntimeError("정의되지 않은 변수 '" + name + "'");
}
void Environment::assign(const std::string& name, FabValue v) {
    auto it = values.find(name);
    if (it != values.end()) { it->second = std::move(v); return; }
    if (parent) { parent->assign(name, std::move(v)); return; }
    throw RuntimeError("정의되지 않은 변수 '" + name + "'");
}
} // namespace codefab
