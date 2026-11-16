#pragma once

#include <okapy/mod_loader.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace okapy {

enum class ModLifecycleState {
    Registered,
    Initializing,
    Running,
    Stopping,
    Stopped,
    Failed
};

enum class LifecycleResult {
    Registered,
    Started,
    AlreadyRunning,
    InvalidId,
    DuplicateId,
    MissingInitializer,
    MissingModule,
    InvalidLoadPlan,
    InitializationFailed,
    ShutdownFailed
};

struct ModLifecycleInfo {
    std::string id;
    ModLifecycleState state = ModLifecycleState::Registered;
    std::string error;
};

class ModLifecycleManager final {
public:
    using Initializer = std::function<bool()>;
    using Shutdown = std::function<void()>;

    ModLifecycleManager() = default;
    ~ModLifecycleManager();

    ModLifecycleManager(const ModLifecycleManager&) = delete;
    ModLifecycleManager& operator=(const ModLifecycleManager&) = delete;

    // Callbacks are supplied by the host after validating the mod and its ABI.
    LifecycleResult registerModule(
        std::string id, Initializer initialize, Shutdown shutdown = {});

    // Starts modules in the load plan order. On failure, previously started
    // modules are shut down in reverse order.
    LifecycleResult start(const ModLoadPlan& plan);

    // Stops all running modules in reverse start order.
    LifecycleResult stopAll() noexcept;

    [[nodiscard]] std::vector<ModLifecycleInfo> list() const;
    [[nodiscard]] std::size_t runningCount() const noexcept;
    [[nodiscard]] bool isRunning(std::string_view id) const noexcept;

private:
    struct Entry {
        Initializer initialize;
        Shutdown shutdown;
        ModLifecycleState state = ModLifecycleState::Registered;
        std::string error;
    };

    std::unordered_map<std::string, Entry> m_modules;
    std::vector<std::string> m_startOrder;
};

} // namespace okapy
