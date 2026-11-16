#pragma once

#include <okapy/mod_registry.hpp>

#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace okapy {

inline constexpr std::string_view kPermissionConfigRead = "config.read";
inline constexpr std::string_view kPermissionConfigWrite = "config.write";
inline constexpr std::string_view kPermissionEventsSubscribe = "events.subscribe";
inline constexpr std::string_view kPermissionEventsPublish = "events.publish";
inline constexpr std::string_view kPermissionGameInfo = "game.info";
inline constexpr std::string_view kPermissionLoggingWrite = "logging.write";

class PermissionManager final {
public:
    bool grant(std::string_view modId, std::string_view permission) {
        if (!ModRegistry::isValidId(modId) || !isKnownPermission(permission)) return false;
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_permissions[std::string(modId)].insert(std::string(permission)).second;
    }

    bool revoke(std::string_view modId, std::string_view permission) {
        if (!ModRegistry::isValidId(modId) || !isKnownPermission(permission)) return false;
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto found = m_permissions.find(std::string(modId));
        if (found == m_permissions.end()) return false;
        const bool removed = found->second.erase(std::string(permission)) != 0;
        if (found->second.empty()) m_permissions.erase(found);
        return removed;
    }

    [[nodiscard]] bool has(std::string_view modId, std::string_view permission) const {
        if (!ModRegistry::isValidId(modId) || !isKnownPermission(permission)) return false;
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto found = m_permissions.find(std::string(modId));
        return found != m_permissions.end() &&
            found->second.find(std::string(permission)) != found->second.end();
    }

    [[nodiscard]] std::vector<std::string> list(std::string_view modId) const {
        std::vector<std::string> result;
        if (!ModRegistry::isValidId(modId)) return result;
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto found = m_permissions.find(std::string(modId));
        if (found == m_permissions.end()) return result;
        result.assign(found->second.begin(), found->second.end());
        return result;
    }

    void clearMod(std::string_view modId) {
        if (!ModRegistry::isValidId(modId)) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_permissions.erase(std::string(modId));
    }

    static bool isKnownPermission(std::string_view permission) noexcept {
        return permission == kPermissionConfigRead ||
            permission == kPermissionConfigWrite ||
            permission == kPermissionEventsSubscribe ||
            permission == kPermissionEventsPublish ||
            permission == kPermissionGameInfo ||
            permission == kPermissionLoggingWrite;
    }

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::unordered_set<std::string>> m_permissions;
};

} // namespace okapy
