#pragma once

#include <cstddef>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace okapy {

enum class ModState {
    Discovered,
    Installed,
    Enabled,
    Disabled,
    Error
};

struct ModInfo {
    std::string id;
    std::string name;
    std::string version;
    std::string developer;
    std::string packagePath;
    ModState state = ModState::Discovered;
};

enum class RegistryResult {
    Added,
    InvalidId,
    DuplicateId
};

// Thread-safe in-memory registry. It stores metadata only; it never loads code.
class ModRegistry final {
public:
    [[nodiscard]] static bool isValidId(std::string_view id) noexcept;

    RegistryResult add(ModInfo mod);
    bool remove(std::string_view id);
    bool replace(ModInfo mod);
    bool setState(std::string_view id, ModState state);

    [[nodiscard]] std::optional<ModInfo> find(std::string_view id) const;
    [[nodiscard]] std::vector<ModInfo> list() const;
    [[nodiscard]] std::size_t size() const;
    void clear();

private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, ModInfo> m_mods;
};

} // namespace okapy
