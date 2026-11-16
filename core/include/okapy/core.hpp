#pragma once

#include <okapy/config_store.hpp>
#include <okapy/event_system.hpp>
#include <okapy/hook_manager.hpp>
#include <okapy/installed_mod_catalog.hpp>
#include <okapy/mod_abi.hpp>
#include <okapy/mod_lifecycle.hpp>
#include <okapy/mod_loader.hpp>
#include <okapy/mod_registry.hpp>
#include <okapy/native_mod_loader.hpp>
#include <okapy/native_module.hpp>
#include <okapy/patch_manager.hpp>
#include <okapy/permission_manager.hpp>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace okapy {

inline constexpr std::string_view kName = "Okapy";
inline constexpr std::string_view kVersion = "0.00.1";

struct ModEvent final {
    std::string name;
    std::string payload;
};

class Core final {
public:
    bool initialize() noexcept;
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] ModRegistry& mods() noexcept;
    [[nodiscard]] InstalledModCatalog& installedMods() noexcept;
    [[nodiscard]] const InstalledModCatalog& installedMods() const noexcept;
    [[nodiscard]] const ModRegistry& mods() const noexcept;
    [[nodiscard]] EventBus& events() noexcept;
    [[nodiscard]] const EventBus& events() const noexcept;
    [[nodiscard]] HookManager& hooks() noexcept;
    [[nodiscard]] const HookManager& hooks() const noexcept;
    [[nodiscard]] PatchManager& patches() noexcept;
    [[nodiscard]] const PatchManager& patches() const noexcept;
    [[nodiscard]] ModLoader& loader() noexcept;
    [[nodiscard]] const ModLoader& loader() const noexcept;
    [[nodiscard]] ConfigStore& config() noexcept;
    [[nodiscard]] const ConfigStore& config() const noexcept;
    [[nodiscard]] PermissionManager& permissions() noexcept;
    [[nodiscard]] const PermissionManager& permissions() const noexcept;
    [[nodiscard]] OkapyHostApiV1 hostApi() noexcept;
    [[nodiscard]] NativeModLoader& nativeMods() noexcept;
    [[nodiscard]] const NativeModLoader& nativeMods() const noexcept;
    [[nodiscard]] ModLifecycleManager& lifecycle() noexcept;
    [[nodiscard]] const ModLifecycleManager& lifecycle() const noexcept;

private:
    static int32_t configGetCallback(void* context, const char* modId,
        const char* key, char* buffer, uint32_t capacity,
        uint32_t* requiredSize) noexcept;
    static int32_t configSetCallback(void* context, const char* modId,
        const char* key, const char* value) noexcept;
    static int32_t eventSubscribeCallback(void* context, const char* modId,
        const char* eventName, OkapyEventCallbackV1 callback, void* userData,
        uint64_t* outToken) noexcept;
    static int32_t eventUnsubscribeCallback(void* context, const char* modId,
        uint64_t token) noexcept;
    static int32_t eventPublishCallback(void* context, const char* modId,
        const char* eventName, const char* payload, uint32_t* outListenerCount) noexcept;
    static int32_t permissionCheckCallback(void* context, const char* modId,
        const char* permission) noexcept;

    int32_t subscribeEvent(const char* modId, const char* eventName,
        OkapyEventCallbackV1 callback, void* userData, uint64_t* outToken) noexcept;
    int32_t unsubscribeEvent(const char* modId, uint64_t token) noexcept;
    int32_t publishEvent(const char* modId, const char* eventName,
        const char* payload, uint32_t* outListenerCount) noexcept;

    struct EventSubscription {
        std::string ownerId;
        std::string eventName;
        ListenerHandle handle;
    };

    bool m_initialized = false;
    ModRegistry m_modRegistry;
    InstalledModCatalog m_installedModCatalog{m_modRegistry};
    EventBus m_eventBus;
    std::mutex m_eventSubscriptionsMutex;
    std::unordered_map<uint64_t, EventSubscription> m_eventSubscriptions;
    std::atomic<uint64_t> m_nextEventToken{1};
    HookManager m_hookManager;
    PatchManager m_patchManager;
    ModLoader m_modLoader;
    ConfigStore m_configStore;
    PermissionManager m_permissionManager;
    NativeModLoader m_nativeModLoader;
    // Lifecycle is destroyed before NativeModLoader so callbacks run while libraries remain loaded.
    ModLifecycleManager m_lifecycleManager;
};

} // namespace okapy
