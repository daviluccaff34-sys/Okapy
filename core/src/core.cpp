#include <okapy/core.hpp>

#include <limits>
#include <string>

namespace okapy {

bool Core::initialize() noexcept { m_initialized = true; return true; }
bool Core::isInitialized() const noexcept { return m_initialized; }
ModRegistry& Core::mods() noexcept { return m_modRegistry; }
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
NativeModLoader& Core::nativeMods() noexcept { return m_nativeModLoader; }
const NativeModLoader& Core::nativeMods() const noexcept { return m_nativeModLoader; }
ModLifecycleManager& Core::lifecycle() noexcept { return m_lifecycleManager; }
const ModLifecycleManager& Core::lifecycle() const noexcept { return m_lifecycleManager; }

int32_t Core::configGetCallback(void* context, const char* modId,
    const char* key, char* buffer, uint32_t capacity,
    uint32_t* requiredSize) noexcept {
    if (!context) return OKAPY_CONFIG_ERROR;
    auto* core = static_cast<Core*>(context);
    auto api = core->m_configStore.hostApi();
    return api.config_get
        ? api.config_get(api.host_context, modId, key, buffer, capacity, requiredSize)
        : OKAPY_CONFIG_ERROR;
}

int32_t Core::configSetCallback(void* context, const char* modId,
    const char* key, const char* value) noexcept {
    if (!context) return OKAPY_CONFIG_ERROR;
    auto* core = static_cast<Core*>(context);
    auto api = core->m_configStore.hostApi();
    return api.config_set
        ? api.config_set(api.host_context, modId, key, value)
        : OKAPY_CONFIG_ERROR;
}

int32_t Core::eventSubscribeCallback(void* context, const char* eventName,
    OkapyEventCallbackV1 callback, void* userData,
    uint64_t* outToken) noexcept {
    if (!context) return OKAPY_EVENT_ERROR;
    return static_cast<Core*>(context)->subscribeEvent(
        eventName, callback, userData, outToken);
}

int32_t Core::eventUnsubscribeCallback(void* context, uint64_t token) noexcept {
    if (!context) return OKAPY_EVENT_ERROR;
    return static_cast<Core*>(context)->unsubscribeEvent(token);
}

int32_t Core::eventPublishCallback(void* context, const char* eventName,
    const char* payload, uint32_t* outListenerCount) noexcept {
    if (!context) return OKAPY_EVENT_ERROR;
    return static_cast<Core*>(context)->publishEvent(
        eventName, payload, outListenerCount);
}

int32_t Core::subscribeEvent(const char* eventName,
    OkapyEventCallbackV1 callback, void* userData,
    uint64_t* outToken) noexcept {
    if (!eventName || !*eventName || !callback || !outToken)
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
            m_eventSubscriptions.emplace(token, std::move(handle));
        }
        *outToken = token;
        return OKAPY_EVENT_OK;
    } catch (...) {
        return OKAPY_EVENT_ERROR;
    }
}

int32_t Core::unsubscribeEvent(uint64_t token) noexcept {
    if (token == 0) return OKAPY_EVENT_INVALID_ARGUMENT;
    try {
        std::lock_guard<std::mutex> lock(m_eventSubscriptionsMutex);
        const auto found = m_eventSubscriptions.find(token);
        if (found == m_eventSubscriptions.end()) return OKAPY_EVENT_NOT_FOUND;
        found->second.reset();
        m_eventSubscriptions.erase(found);
        return OKAPY_EVENT_OK;
    } catch (...) {
        return OKAPY_EVENT_ERROR;
    }
}

int32_t Core::publishEvent(const char* eventName, const char* payload,
    uint32_t* outListenerCount) noexcept {
    if (!eventName || !*eventName || !payload)
        return OKAPY_EVENT_INVALID_ARGUMENT;
    try {
        const auto count = m_eventBus.publish(ModEvent{eventName, payload});
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
    return api;
}

} // namespace okapy
