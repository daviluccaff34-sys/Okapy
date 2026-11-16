#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

namespace okapy {

enum class PatchState {
    Registered,
    Enabled,
    Disabled
};

struct PatchInfo {
    std::string id;
    std::string owner;
    std::uintptr_t targetAddress = 0;
    std::vector<std::uint8_t> expectedBytes;
    std::vector<std::uint8_t> replacementBytes;
    PatchState state = PatchState::Registered;
};

enum class PatchResult {
    Added,
    InvalidId,
    InvalidOwner,
    InvalidAddress,
    EmptyPatch,
    SizeMismatch,
    AddressOverflow,
    DuplicateId,
    Overlap
};

// Validates and tracks patch descriptions only. It never reads or writes process memory.
class PatchManager final {
public:
    [[nodiscard]] static bool isValidId(std::string_view id) noexcept;

    PatchResult add(PatchInfo patch);
    bool remove(std::string_view id);
    bool setEnabled(std::string_view id, bool enabled);
    std::size_t disableOwner(std::string_view owner);
    [[nodiscard]] std::optional<PatchInfo> find(std::string_view id) const;
    [[nodiscard]] std::vector<PatchInfo> list() const;
    [[nodiscard]] std::size_t size() const;
    void clear();

private:
    mutable std::shared_mutex m_mutex;
    std::vector<PatchInfo> m_patches;
};

} // namespace okapy
