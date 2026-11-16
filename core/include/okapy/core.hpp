#pragma once

#include <okapy/event_system.hpp>
#include <okapy/hook_manager.hpp>
#include <okapy/mod_registry.hpp>

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

private:
    bool m_initialized = false;
    ModRegistry m_modRegistry;
    EventBus m_eventBus;
    HookManager m_hookManager;
};

} // namespace okapy
