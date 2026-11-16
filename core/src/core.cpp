#include <okapy/core.hpp>

namespace okapy {
namespace {
bool g_initialized = false;
}

bool Core::initialize() noexcept {
    // Placeholder for deterministic core startup.
    // Future stages will initialize package, dependency, event and runtime systems.
    g_initialized = true;
    return g_initialized;
}

bool Core::isInitialized() const noexcept {
    return g_initialized;
}

} // namespace okapy
