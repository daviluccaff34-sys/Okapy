#include <okapy/mod_registry.hpp>

#include <algorithm>
#include <mutex>
#include <utility>

namespace okapy {

bool ModRegistry::isValidId(std::string_view id) noexcept {
    if (id.empty() || id.size() > 64) {
        return false;
    }

    bool segmentHasCharacter = false;
    unsigned int dotCount = 0;

    for (const unsigned char ch : id) {
        if (ch == '.') {
            if (!segmentHasCharacter || ++dotCount > 1) {
                return false;
            }
            segmentHasCharacter = false;
            continue;
        }

        const bool valid =
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '_' || ch == '-';

        if (!valid) {
            return false;
        }
        segmentHasCharacter = true;
    }

    return segmentHasCharacter;
}

RegistryResult ModRegistry::add(ModInfo mod) {
    if (!isValidId(mod.id)) {
        return RegistryResult::InvalidId;
    }

    std::unique_lock lock(m_mutex);
    const auto id = mod.id;
    const auto [it, inserted] = m_mods.emplace(id, std::move(mod));
    (void)it;
    return inserted ? RegistryResult::Added : RegistryResult::DuplicateId;
}

bool ModRegistry::remove(std::string_view id) {
    std::unique_lock lock(m_mutex);
    return m_mods.erase(std::string(id)) != 0;
}

bool ModRegistry::setState(std::string_view id, ModState state) {
    std::unique_lock lock(m_mutex);
    const auto it = m_mods.find(std::string(id));
    if (it == m_mods.end()) {
        return false;
    }
    it->second.state = state;
    return true;
}

std::optional<ModInfo> ModRegistry::find(std::string_view id) const {
    std::shared_lock lock(m_mutex);
    const auto it = m_mods.find(std::string(id));
    if (it == m_mods.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<ModInfo> ModRegistry::list() const {
    std::shared_lock lock(m_mutex);
    std::vector<ModInfo> result;
    result.reserve(m_mods.size());
    for (const auto& entry : m_mods) {
        result.push_back(entry.second);
    }
    std::sort(result.begin(), result.end(),
        [](const ModInfo& left, const ModInfo& right) {
            return left.id < right.id;
        });
    return result;
}

std::size_t ModRegistry::size() const noexcept {
    std::shared_lock lock(m_mutex);
    return m_mods.size();
}

void ModRegistry::clear() {
    std::unique_lock lock(m_mutex);
    m_mods.clear();
}

} // namespace okapy
