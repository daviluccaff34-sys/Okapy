#pragma once

#include <okapy/config_store.hpp>
#include <okapy/event_system.hpp>
#include <okapy/hook_manager.hpp>
#include <okapy/mod_abi.hpp>
#include <okapy/mod_lifecycle.hpp>
#include <okapy/mod_loader.hpp>
#include <okapy/mod_registry.hpp>
#include <okapy/native_mod_loader.hpp>
#include <okapy/native_module.hpp>
#include <okapy/patch_manager.hpp>

#include <string_view>

namespace okapy {
inline constexpr std::string_view kName = "Okapy";
inline constexpr std::string_view kVersion = "0.00.1";
class Core final {
public:
    bool initialize() noexcept;
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] ModRegistry& mods() noexcept;
    [[nodiscard]] const ModRegistry& mods() const noexcept;
    [[nodiscard]] EventBus& events() noexcept;
    [[nodiscard]] const EventBus& events() const noexcept;
    [[nodiscard]] HookManager& hooks() noexcept;
    [[nodiscard]] const HookManager& hooks() const noexcept;
    [[nodiscard]] PatchManager& patches() noexcept;
    [[nodiscard]] const PatchManager& patches() const noexcept;
    [[nodiscard]] ModLoader& loader() noexcept;
    [[nodiscard]] const ModLoader& loader() const noexcept;
    [[nodiscard]] ConfigStore& config() noexcept;
    [[nodiscard]] const ConfigStore& config() const noexcept;
    [[nodiscard]] OkapyHostApiV1 hostApi() noexcept;
    [[nodiscard]] NativeModLoader& nativeMods() noexcept;
    [[nodiscard]] const NativeModLoader& nativeMods() const noexcept;
    [[nodiscard]] ModLifecycleManager& lifecycle() noexcept;
    [[nodiscard]] const ModLifecycleManager& lifecycle() const noexcept;
private:
    bool m_initialized = false;
    ModRegistry m_modRegistry;
    EventBus m_eventBus;
    HookManager m_hookManager;
    PatchManager m_patchManager;
    ModLoader m_modLoader;
    ConfigStore m_configStore;
    NativeModLoader m_nativeModLoader;
    // Lifecycle is destroyed before NativeModLoader so callbacks run while libraries remain loaded.
    ModLifecycleManager m_lifecycleManager;
};
} // namespace okapy