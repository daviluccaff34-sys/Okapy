#pragma once

#include <okapy/mod_registry.hpp>

#include <string_view>

namespace okapy {

inline constexpr std::string_view kName = "Okapy";
inline constexpr std::string_view kVersion = "0.00.1";

class Core final {
public:
    // Initializes the standalone core foundation; does not load game code.
    bool initialize() noexcept;

    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] ModRegistry& mods() noexcept;
    [[nodiscard]] const ModRegistry& mods() const noexcept;

private:
    bool m_initialized = false;
    ModRegistry m_modRegistry;
};

} // namespace okapy
