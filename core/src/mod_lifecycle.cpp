#include <okapy/mod_lifecycle.hpp>

#include <algorithm>
#include <exception>
#include <utility>

namespace okapy {

ModLifecycleManager::~ModLifecycleManager() {
    stopAll();
}

LifecycleResult ModLifecycleManager::registerModule(
    std::string id, Initializer initialize, Shutdown shutdown) {
    if (!ModRegistry::isValidId(id)) return LifecycleResult::InvalidId;
    if (!initialize) return LifecycleResult::MissingInitializer;
    if (m_modules.find(id) != m_modules.end()) return LifecycleResult::DuplicateId;

    m_modules.emplace(std::move(id),
        Entry{std::move(initialize), std::move(shutdown),
              ModLifecycleState::Registered, {}});
    return LifecycleResult::Registered;
}

LifecycleResult ModLifecycleManager::start(const ModLoadPlan& plan) {
    if (!plan.isReady()) return LifecycleResult::InvalidLoadPlan;
    if (!m_startOrder.empty()) return LifecycleResult::AlreadyRunning;

    for (const auto& id : plan.dependencies.loadOrder) {
        if (m_modules.find(id) == m_modules.end()) {
            return LifecycleResult::MissingModule;
        }
    }

    for (const auto& id : plan.dependencies.loadOrder) {
        auto& entry = m_modules.at(id);
        entry.state = ModLifecycleState::Initializing;
        entry.error.clear();

        bool initialized = false;
        try {
            initialized = entry.initialize();
        } catch (const std::exception& exception) {
            entry.error = exception.what();
        } catch (...) {
            entry.error = "Mod initializer threw an unknown exception.";
        }

        if (!initialized) {
            if (entry.error.empty()) {
                entry.error = "Mod initializer returned false.";
            }
            entry.state = ModLifecycleState::Failed;
            try {
                if (entry.shutdown) entry.shutdown();
            } catch (const std::exception& exception) {
                entry.error += " Shutdown callback: ";
                entry.error += exception.what();
            } catch (...) {
                entry.error += " Shutdown callback threw an unknown exception.";
            }
            stopAll();
            return LifecycleResult::InitializationFailed;
        }

        entry.state = ModLifecycleState::Running;
        m_startOrder.push_back(id);
    }

    return LifecycleResult::Started;
}

LifecycleResult ModLifecycleManager::stopAll() noexcept {
    bool shutdownFailed = false;

    while (!m_startOrder.empty()) {
        const auto id = std::move(m_startOrder.back());
        m_startOrder.pop_back();

        const auto found = m_modules.find(id);
        if (found == m_modules.end()) continue;

        auto& entry = found->second;
        entry.state = ModLifecycleState::Stopping;
        try {
            if (entry.shutdown) entry.shutdown();
            entry.state = ModLifecycleState::Stopped;
        } catch (const std::exception& exception) {
            entry.error = exception.what();
            entry.state = ModLifecycleState::Failed;
            shutdownFailed = true;
        } catch (...) {
            entry.error = "Mod shutdown callback threw an unknown exception.";
            entry.state = ModLifecycleState::Failed;
            shutdownFailed = true;
        }
    }

    return shutdownFailed ? LifecycleResult::ShutdownFailed : LifecycleResult::Started;
}

std::vector<ModLifecycleInfo> ModLifecycleManager::list() const {
    std::vector<ModLifecycleInfo> result;
    result.reserve(m_modules.size());
    for (const auto& item : m_modules) {
        result.push_back({item.first, item.second.state, item.second.error});
    }
    std::sort(result.begin(), result.end(),
        [](const auto& left, const auto& right) { return left.id < right.id; });
    return result;
}

std::size_t ModLifecycleManager::runningCount() const noexcept {
    return m_startOrder.size();
}

bool ModLifecycleManager::isRunning(std::string_view id) const noexcept {
    const auto found = m_modules.find(std::string(id));
    return found != m_modules.end() &&
           found->second.state == ModLifecycleState::Running;
}

} // namespace okapy
