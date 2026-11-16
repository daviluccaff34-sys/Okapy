#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace okapy {

namespace detail {

struct EventListenerBase {
    EventListenerBase(std::size_t listenerId, int listenerPriority)
        : id(listenerId), priority(listenerPriority) {}
    virtual ~EventListenerBase() = default;

    std::size_t id;
    int priority;
    std::atomic<bool> active{true};
};

template <typename Event>
struct EventListener final : EventListenerBase {
    using Callback = std::function<void(const Event&)>;

    EventListener(std::size_t id, int priority, Callback callback)
        : EventListenerBase(id, priority), callback(std::move(callback)) {}

    Callback callback;
};

struct EventBusState {
    std::mutex mutex;
    std::unordered_map<std::type_index,
        std::vector<std::shared_ptr<EventListenerBase>>> listeners;
    std::atomic<std::size_t> nextId{1};
};

} // namespace detail

class EventBus;

class ListenerHandle final {
public:
    ListenerHandle() = default;
    ~ListenerHandle() { reset(); }

    ListenerHandle(const ListenerHandle&) = delete;
    ListenerHandle& operator=(const ListenerHandle&) = delete;

    ListenerHandle(ListenerHandle&& other) noexcept
        : m_state(std::move(other.m_state)),
          m_type(other.m_type),
          m_id(std::exchange(other.m_id, 0)) {}

    ListenerHandle& operator=(ListenerHandle&& other) noexcept {
        if (this != &other) {
            reset();
            m_state = std::move(other.m_state);
            m_type = other.m_type;
            m_id = std::exchange(other.m_id, 0);
        }
        return *this;
    }

    void reset() noexcept {
        if (m_id == 0) return;
        if (auto state = m_state.lock()) {
            std::lock_guard<std::mutex> lock(state->mutex);
            const auto found = state->listeners.find(m_type);
            if (found != state->listeners.end()) {
                auto& entries = found->second;
                for (auto& entry : entries) {
                    if (entry->id == m_id) {
                        entry->active.store(false, std::memory_order_release);
                        break;
                    }
                }
                entries.erase(
                    std::remove_if(entries.begin(), entries.end(),
                        [this](const auto& entry) { return entry->id == m_id; }),
                    entries.end());
                if (entries.empty()) state->listeners.erase(found);
            }
        }
        m_id = 0;
        m_state.reset();
    }

    [[nodiscard]] bool isActive() const noexcept {
        return m_id != 0 && !m_state.expired();
    }

private:
    friend class EventBus;
    ListenerHandle(std::weak_ptr<detail::EventBusState> state,
                   std::type_index type, std::size_t id)
        : m_state(std::move(state)), m_type(type), m_id(id) {}

    std::weak_ptr<detail::EventBusState> m_state;
    std::type_index m_type{typeid(void)};
    std::size_t m_id = 0;
};

class EventBus final {
public:
    EventBus() : m_state(std::make_shared<detail::EventBusState>()) {}
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
    EventBus(EventBus&&) = delete;
    EventBus& operator=(EventBus&&) = delete;

    template <typename Event, typename Callback>
    ListenerHandle subscribe(Callback&& callback, int priority = 0) {
        using Slot = detail::EventListener<Event>;
        const auto id = m_state->nextId.fetch_add(1, std::memory_order_relaxed);
        auto slot = std::make_shared<Slot>(
            id, priority, typename Slot::Callback(std::forward<Callback>(callback)));

        {
            std::lock_guard<std::mutex> lock(m_state->mutex);
            m_state->listeners[std::type_index(typeid(Event))].push_back(slot);
        }
        return ListenerHandle{m_state, std::type_index(typeid(Event)), id};
    }

    template <typename Event>
    std::size_t publish(const Event& event) const {
        using Slot = detail::EventListener<Event>;
        std::vector<std::shared_ptr<Slot>> snapshot;

        {
            std::lock_guard<std::mutex> lock(m_state->mutex);
            const auto found = m_state->listeners.find(std::type_index(typeid(Event)));
            if (found == m_state->listeners.end()) return 0;
            snapshot.reserve(found->second.size());
            for (const auto& base : found->second) {
                snapshot.push_back(std::static_pointer_cast<Slot>(base));
            }
        }

        std::stable_sort(snapshot.begin(), snapshot.end(),
            [](const auto& left, const auto& right) {
                return left->priority > right->priority;
            });

        std::size_t invoked = 0;
        for (const auto& slot : snapshot) {
            if (!slot->active.load(std::memory_order_acquire)) continue;
            slot->callback(event);
            ++invoked;
        }
        return invoked;
    }

    template <typename Event>
    void clear() {
        std::lock_guard<std::mutex> lock(m_state->mutex);
        const auto found = m_state->listeners.find(std::type_index(typeid(Event)));
        if (found == m_state->listeners.end()) return;
        for (const auto& slot : found->second) {
            slot->active.store(false, std::memory_order_release);
        }
        m_state->listeners.erase(found);
    }

    void clearAll() {
        std::lock_guard<std::mutex> lock(m_state->mutex);
        for (auto& group : m_state->listeners) {
            for (const auto& slot : group.second) {
                slot->active.store(false, std::memory_order_release);
            }
        }
        m_state->listeners.clear();
    }

    [[nodiscard]] std::size_t listenerCount() const {
        std::lock_guard<std::mutex> lock(m_state->mutex);
        std::size_t count = 0;
        for (const auto& group : m_state->listeners) count += group.second.size();
        return count;
    }

private:
    std::shared_ptr<detail::EventBusState> m_state;
};

} // namespace okapy
