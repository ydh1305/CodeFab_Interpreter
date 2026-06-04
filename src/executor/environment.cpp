#include "codefab/executor/environment.h"
namespace codefab {
Environment::Environment(std::shared_ptr<Environment> p) : m_parent(std::move(p)) {}
void Environment::define(const std::string& name, FabValue v) { m_values[name] = std::move(v); }
FabValue Environment::get(const std::string& name) const {
    auto it = m_values.find(name);
    if (it != m_values.end()) return it->second;
    if (m_parent) return m_parent->get(name);
    throw RuntimeError("정의되지 않은 변수 '" + name + "'");
}
void Environment::assign(const std::string& name, FabValue v) {
    auto it = m_values.find(name);
    if (it != m_values.end()) { it->second = std::move(v); return; }
    if (m_parent) { m_parent->assign(name, std::move(v)); return; }
    throw RuntimeError("정의되지 않은 변수 '" + name + "'");
}
} // namespace codefab
