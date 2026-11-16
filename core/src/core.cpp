#include <okapy/core.hpp>

namespace okapy {

bool Core::initialize() noexcept {
    m_initialized = true;
    return true;
}

bool Core::isInitialized() const noexcept {
    return m_initialized;
}

ModRegistry& Core::mods() noexcept {
    return m_modRegistry;
}

const ModRegistry& Core::mods() const noexcept {
    return m_modRegistry;
}

} // namespace okapy
