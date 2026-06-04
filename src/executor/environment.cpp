#include "codefab/executor/environment.h"

namespace codefab {

Environment::Environment(std::shared_ptr<Environment> enclosing)
    : m_parent(std::move(enclosing)) {}

void Environment::define(const std::string& name, FabValue value) {
    m_values[name] = std::move(value);
}

FabValue Environment::get(const std::string& name) const {
    auto it = m_values.find(name);
    if (it != m_values.end()) return it->second;
    if (m_parent) return m_parent->get(name);
    throw RuntimeError("정의되지 않은 변수 '" + name + "'");
}

void Environment::assign(const std::string& name, FabValue value) {
    auto it = m_values.find(name);
    if (it != m_values.end()) { it->second = std::move(value); return; }
    if (m_parent) { m_parent->assign(name, std::move(value)); return; }
    throw RuntimeError("정의되지 않은 변수 '" + name + "'");
}

} // namespace codefab
