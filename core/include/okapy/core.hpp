#pragma once

#include <string_view>

namespace okapy {

// Public identity/version surface for the native core.
inline constexpr std::string_view kName = "Okapy";
inline constexpr std::string_view kVersion = "0.00.1";

class Core final {
public:
    // Initializes the standalone core foundation.
    // Game integration and mod loading are intentionally not performed yet.
    bool initialize() noexcept;

    [[nodiscard]] bool isInitialized() const noexcept;
};

} // namespace okapy
