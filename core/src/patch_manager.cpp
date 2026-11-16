#include <okapy/patch_manager.hpp>

#include <algorithm>
#include <limits>
#include <mutex>

namespace okapy {

bool PatchManager::isValidId(std::string_view id) noexcept {
    if (id.empty() || id.size() > 64) return false;

    bool previousDot = true;
    unsigned int dots = 0;
    for (const char raw : id) {
        const auto c = static_cast<unsigned char>(raw);
        if (c == '.') {
            if (previousDot || ++dots > 1) return false;
            previousDot = true;
            continue;
        }
        if (!((c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return false;
        }
        previousDot = false;
    }
    return !previousDot;
}

PatchResult PatchManager::add(PatchInfo patch) {
    if (!isValidId(patch.id)) return PatchResult::InvalidId;
    if (!isValidId(patch.owner)) return PatchResult::InvalidOwner;
    if (patch.targetAddress == 0) return PatchResult::InvalidAddress;
    if (patch.expectedBytes.empty() || patch.replacementBytes.empty()) {
        return PatchResult::EmptyPatch;
    }
    if (patch.expectedBytes.size() != patch.replacementBytes.size()) {
        return PatchResult::SizeMismatch;
    }

    const auto maxAddress = std::numeric_limits<std::uintptr_t>::max();
    const auto length = static_cast<std::uintptr_t>(patch.expectedBytes.size());
    if (length > maxAddress - patch.targetAddress) {
        return PatchResult::AddressOverflow;
    }
    const auto patchEnd = patch.targetAddress + length;

    std::unique_lock<std::shared_mutex> lock(m_mutex);
    for (const auto& existing : m_patches) {
        if (existing.id == patch.id) return PatchResult::DuplicateId;

        const auto existingLength =
            static_cast<std::uintptr_t>(existing.expectedBytes.size());
        const auto existingEnd = existing.targetAddress + existingLength;
        if (patch.targetAddress < existingEnd &&
            existing.targetAddress < patchEnd) {
            return PatchResult::Overlap;
        }
    }

    patch.state = PatchState::Registered;
    m_patches.push_back(std::move(patch));
    return PatchResult::Added;
}

bool PatchManager::remove(std::string_view id) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    const auto found = std::find_if(m_patches.begin(), m_patches.end(),
        [id](const PatchInfo& item) { return item.id == id; });
    if (found == m_patches.end()) return false;
    m_patches.erase(found);
    return true;
}

bool PatchManager::setEnabled(std::string_view id, bool enabled) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    const auto found = std::find_if(m_patches.begin(), m_patches.end(),
        [id](const PatchInfo& item) { return item.id == id; });
    if (found == m_patches.end()) return false;
    found->state = enabled ? PatchState::Enabled : PatchState::Disabled;
    return true;
}

std::size_t PatchManager::disableOwner(std::string_view owner) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    std::size_t changed = 0;
    for (auto& patch : m_patches) {
        if (patch.owner == owner && patch.state == PatchState::Enabled) {
            patch.state = PatchState::Disabled;
            ++changed;
        }
    }
    return changed;
}

std::optional<PatchInfo> PatchManager::find(std::string_view id) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    const auto found = std::find_if(m_patches.begin(), m_patches.end(),
        [id](const PatchInfo& item) { return item.id == id; });
    if (found == m_patches.end()) return std::nullopt;
    return *found;
}

std::vector<PatchInfo> PatchManager::list() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    auto result = m_patches;
    std::sort(result.begin(), result.end(),
        [](const PatchInfo& left, const PatchInfo& right) {
            return left.id < right.id;
        });
    return result;
}

std::size_t PatchManager::size() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_patches.size();
}

void PatchManager::clear() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_patches.clear();
}

} // namespace okapy
