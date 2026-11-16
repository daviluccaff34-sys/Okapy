#pragma once

#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

namespace okapy {

enum class HookState {
    Registered,
    Enabled,
    Disabled
};

struct HookInfo {
    std::string id;
    std::string owner;
    std::uintptr_t targetAddress = 0;
    std::uintptr_t detourAddress = 0;
    HookState state = HookState::Registered;
};

enum class HookResult {
    Added,
    InvalidId,
    InvalidOwner,
    InvalidAddress,
    DuplicateId,
    TargetConflict
};

// Tracks hook metadata and requested state only. It does not patch executable memory.
class HookManager final {
public:
    [[nodiscard]] static bool isValidId(std::string_view id) noexcept;

    HookResult add(HookInfo hook);
    bool remove(std::string_view id);
    bool setEnabled(std::string_view id, bool enabled);
    std::size_t disableOwner(std::string_view owner);
    [[nodiscard]] std::optional<HookInfo> find(std::string_view id) const;
    [[nodiscard]] std::vector<HookInfo> list() const;
    [[nodiscard]] std::size_t size() const;
    void clear();

private:
    mutable std::shared_mutex m_mutex;
    std::vector<HookInfo> m_hooks;
};

} // namespace okapy
