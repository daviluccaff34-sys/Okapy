#include <okapy/hook_manager.hpp>

#include <algorithm>
#include <mutex>

namespace okapy {

bool HookManager::isValidId(std::string_view id) noexcept {
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

HookResult HookManager::add(HookInfo hook) {
    if (!isValidId(hook.id)) return HookResult::InvalidId;
    if (!isValidId(hook.owner)) return HookResult::InvalidOwner;
    if (hook.targetAddress == 0 || hook.detourAddress == 0) {
        return HookResult::InvalidAddress;
    }

    std::unique_lock<std::shared_mutex> lock(m_mutex);
    const auto duplicate = std::find_if(m_hooks.begin(), m_hooks.end(),
        [&hook](const HookInfo& item) { return item.id == hook.id; });
    if (duplicate != m_hooks.end()) return HookResult::DuplicateId;

    const auto conflict = std::find_if(m_hooks.begin(), m_hooks.end(),
        [&hook](const HookInfo& item) {
            return item.targetAddress == hook.targetAddress;
        });
    if (conflict != m_hooks.end()) return HookResult::TargetConflict;

    hook.state = HookState::Registered;
    m_hooks.push_back(std::move(hook));
    return HookResult::Added;
}

bool HookManager::remove(std::string_view id) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    const auto found = std::find_if(m_hooks.begin(), m_hooks.end(),
        [id](const HookInfo& item) { return item.id == id; });
    if (found == m_hooks.end()) return false;
    m_hooks.erase(found);
    return true;
}

bool HookManager::setEnabled(std::string_view id, bool enabled) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    const auto found = std::find_if(m_hooks.begin(), m_hooks.end(),
        [id](const HookInfo& item) { return item.id == id; });
    if (found == m_hooks.end()) return false;
    found->state = enabled ? HookState::Enabled : HookState::Disabled;
    return true;
}

std::size_t HookManager::disableOwner(std::string_view owner) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    std::size_t changed = 0;
    for (auto& hook : m_hooks) {
        if (hook.owner == owner && hook.state == HookState::Enabled) {
            hook.state = HookState::Disabled;
            ++changed;
        }
    }
    return changed;
}

std::optional<HookInfo> HookManager::find(std::string_view id) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    const auto found = std::find_if(m_hooks.begin(), m_hooks.end(),
        [id](const HookInfo& item) { return item.id == id; });
    if (found == m_hooks.end()) return std::nullopt;
    return *found;
}

std::vector<HookInfo> HookManager::list() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    auto result = m_hooks;
    std::sort(result.begin(), result.end(),
        [](const HookInfo& left, const HookInfo& right) {
            return left.id < right.id;
        });
    return result;
}

std::size_t HookManager::size() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_hooks.size();
}

void HookManager::clear() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_hooks.clear();
}

} // namespace okapy
