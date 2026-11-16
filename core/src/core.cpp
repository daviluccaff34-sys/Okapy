#include <okapy/core.hpp>

namespace okapy {
bool Core::initialize() noexcept { m_initialized = true; return true; }
bool Core::isInitialized() const noexcept { return m_initialized; }
ModRegistry& Core::mods() noexcept { return m_modRegistry; }
const ModRegistry& Core::mods() const noexcept { return m_modRegistry; }
EventBus& Core::events() noexcept { return m_eventBus; }
const EventBus& Core::events() const noexcept { return m_eventBus; }
HookManager& Core::hooks() noexcept { return m_hookManager; }
const HookManager& Core::hooks() const noexcept { return m_hookManager; }
PatchManager& Core::patches() noexcept { return m_patchManager; }
const PatchManager& Core::patches() const noexcept { return m_patchManager; }
ModLoader& Core::loader() noexcept { return m_modLoader; }
const ModLoader& Core::loader() const noexcept { return m_modLoader; }
ConfigStore& Core::config() noexcept { return m_configStore; }
const ConfigStore& Core::config() const noexcept { return m_configStore; }
OkapyHostApiV1 Core::hostApi() noexcept { return m_configStore.hostApi(); }
NativeModLoader& Core::nativeMods() noexcept { return m_nativeModLoader; }
const NativeModLoader& Core::nativeMods() const noexcept { return m_nativeModLoader; }
ModLifecycleManager& Core::lifecycle() noexcept { return m_lifecycleManager; }
const ModLifecycleManager& Core::lifecycle() const noexcept { return m_lifecycleManager; }
} // namespace okapy