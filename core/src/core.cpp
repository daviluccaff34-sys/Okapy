#include <okapy/core.hpp>

#include <limits>
#include <string>

namespace okapy {

bool Core::initialize() noexcept { m_initialized = true; return true; }
bool Core::isInitialized() const noexcept { return m_initialized; }
ModRegistry& Core::mods() noexcept { return m_modRegistry; }
InstalledModCatalog& Core::installedMods() noexcept { return m_installedModCatalog; }
const InstalledModCatalog& Core::installedMods() const noexcept { return m_installedModCatalog; }
const ModRegistry& Core::mods() const noexcept { return m_modRegistry; }
EventBus& Core::events() noexcept { return m_eventBus; }
const EventBus& Core::events() const noexcept { return m_eventBus; }
HookManager& Core::hooks() noexcept { return m_hookManager; }
const HookManager& Core::hooks() const noexcept { return m_hookManager; }
PatchManager& Core::patches() noexcept { return m_patchManager; }
const PatchManager& Core::patches() const noexcept { return m_patchManager; }
ModLoader& Core::loader() noexcept { return m_modLoader; }
const ModLoader& Core::loader() const noexcept { return m_modLoader; }
ConfigStore& Core::config() noexcept { return m_configStore; }
const ConfigStore& Core::config() const noexcept { return m_configStore; }
PermissionManager& Core::permissions() noexcept { return m_permissionManager; }
const PermissionManager& Core::permissions() const noexcept { return m_permissionManager; }
NativeModLoader& Core::nativeMods() noexcept { return m_nativeModLoader; }
const NativeModLoader& Core::nativeMods() const noexcept { return m_nativeModLoader; }
ModLifecycleManager& Core::lifecycle() noexcept { return m_lifecycleManager; }
const ModLifecycleManager& Core::lifecycle() const noexcept { return m_lifecycleManager; }

int32_t Core::configGetCallback(void* context, const char* modId,
    const char* key, char* buffer, uint32_t capacity,
    uint32_t* requiredSize) noexcept {
    if (!context) return OKAPY_CONFIG_ERROR;
    auto* core = static_cast<Core*>(context);
    if (!modId || !core->m_permissionManager.has(modId, kPermissionConfigRead))
        return OKAPY_CONFIG_PERMISSION_DENIED;
    auto api = core->m_configStore.hostApi();
    return api.config_get
        ? api.config_get(api.host_context, modId, key, buffer, capacity, requiredSize)
        : OKAPY_CONFIG_ERROR;
}

int32_t Core::configSetCallback(void* context, const char* modId,
    const char* key, const char* value) noexcept {
    if (!context) return OKAPY_CONFIG_ERROR;
    auto* core = static_cast<Core*>(context);
    if (!modId || !core->m_permissionManager.has(modId, kPermissionConfigWrite))
        return OKAPY_CONFIG_PERMISSION_DENIED;
    auto api = core->m_configStore.hostApi();
    return api.config_set
        ? api.config_set(api.host_context, modId, key, value)
        : OKAPY_CONFIG_ERROR;
}

int32_t Core::eventSubscribeCallback(void* context, const char* modId,
    const char* eventName, OkapyEventCallbackV1 callback, void* userData,
    uint64_t* outToken) noexcept {
    if (!context) return OKAPY_EVENT_ERROR;
    return static_cast<Core*>(context)->subscribeEvent(
        modId, eventName, callback, userData, outToken);
}

int32_t Core::eventUnsubscribeCallback(void* context, const char* modId,
    uint64_t token) noexcept {
    if (!context) return OKAPY_EVENT_ERROR;
    return static_cast<Core*>(context)->unsubscribeEvent(modId, token);
}

int32_t Core::eventPublishCallback(void* context, const char* modId,
    const char* eventName, const char* payload, uint32_t* outListenerCount) noexcept {
    if (!context) return OKAPY_EVENT_ERROR;
    return static_cast<Core*>(context)->publishEvent(
        modId, eventName, payload, outListenerCount);
}

int32_t Core::permissionCheckCallback(void* context, const char* modId,
    const char* permission) noexcept {
    if (!context || !modId || !permission) return -1;
    return static_cast<Core*>(context)->m_permissionManager.has(modId, permission) ? 1 : 0;
}

int32_t Core::subscribeEvent(const char* modId, const char* eventName,
    OkapyEventCallbackV1 callback, void* userData,
    uint64_t* outToken) noexcept {
    if (!modId || !ModRegistry::isValidId(modId) ||
        !m_permissionManager.has(modId, kPermissionEventsSubscribe))
        return OKAPY_EVENT_PERMISSION_DENIED;
    if (!eventName || !*eventName || !callback || !outToken ||
        std::char_traits<char>::length(eventName) > 256)
        return OKAPY_EVENT_INVALID_ARGUMENT;
    try {
        const auto token = m_nextEventToken.fetch_add(1, std::memory_order_relaxed);
        if (token == 0 || token == std::numeric_limits<uint64_t>::max())
            return OKAPY_EVENT_ERROR;

        auto handle = m_eventBus.subscribe<ModEvent>(
            [eventName = std::string(eventName), callback, userData](const ModEvent& event) {
                if (event.name != eventName) return;
                try {
                    callback(userData, event.name.c_str(), event.payload.c_str());
                } catch (...) {
                    // Never allow a foreign callback exception to cross the host boundary.
                }
            });
        {
            std::lock_guard<std::mutex> lock(m_eventSubscriptionsMutex);
            m_eventSubscriptions.emplace(token,
                EventSubscription{modId, eventName, std::move(handle)});
        }
        *outToken = token;
        return OKAPY_EVENT_OK;
    } catch (...) {
        return OKAPY_EVENT_ERROR;
    }
}

int32_t Core::unsubscribeEvent(const char* modId, uint64_t token) noexcept {
    if (!modId || !ModRegistry::isValidId(modId)) return OKAPY_EVENT_INVALID_ARGUMENT;
    if (token == 0) return OKAPY_EVENT_INVALID_ARGUMENT;
    try {
        std::lock_guard<std::mutex> lock(m_eventSubscriptionsMutex);
        const auto found = m_eventSubscriptions.find(token);
        if (found == m_eventSubscriptions.end()) return OKAPY_EVENT_NOT_FOUND;
        if (found->second.ownerId != modId) return OKAPY_EVENT_PERMISSION_DENIED;
        found->second.handle.reset();
        m_eventSubscriptions.erase(found);
        return OKAPY_EVENT_OK;
    } catch (...) {
        return OKAPY_EVENT_ERROR;
    }
}

int32_t Core::publishEvent(const char* modId, const char* eventName, const char* payload,
    uint32_t* outListenerCount) noexcept {
    if (!modId || !ModRegistry::isValidId(modId) ||
        !m_permissionManager.has(modId, kPermissionEventsPublish))
        return OKAPY_EVENT_PERMISSION_DENIED;
    if (!eventName || !*eventName || !payload ||
        std::char_traits<char>::length(eventName) > 256 ||
        std::char_traits<char>::length(payload) > 1024 * 1024)
        return OKAPY_EVENT_INVALID_ARGUMENT;
    try {
        std::size_t matchingListeners = 0;
        {
            std::lock_guard<std::mutex> lock(m_eventSubscriptionsMutex);
            for (const auto& item : m_eventSubscriptions) {
                if (item.second.eventName == eventName) ++matchingListeners;
            }
        }
        m_eventBus.publish(ModEvent{eventName, payload});
        const auto count = matchingListeners;
        if (outListenerCount) {
            *outListenerCount = count > std::numeric_limits<uint32_t>::max()
                ? std::numeric_limits<uint32_t>::max()
                : static_cast<uint32_t>(count);
        }
        return OKAPY_EVENT_OK;
    } catch (...) {
        return OKAPY_EVENT_ERROR;
    }
}

OkapyHostApiV1 Core::hostApi() noexcept {
    auto api = m_configStore.hostApi();
    api.host_context = this;
    api.config_get = &Core::configGetCallback;
    api.config_set = &Core::configSetCallback;
    api.event_subscribe = &Core::eventSubscribeCallback;
    api.event_unsubscribe = &Core::eventUnsubscribeCallback;
    api.event_publish = &Core::eventPublishCallback;
    api.permission_check = &Core::permissionCheckCallback;
    return api;
}

} // namespace okapy
