#include <okapy/event_system.hpp>

#include <cassert>
#include <string>
#include <vector>

namespace {

struct NumberEvent { int value; };
struct TextEvent { std::string value; };

void testTypedDelivery() {
    okapy::EventBus bus;
    int received = 0;
    auto handle = bus.subscribe<NumberEvent>(
        [&received](const NumberEvent& event) { received = event.value; });

    assert(bus.publish(NumberEvent{42}) == 1);
    assert(received == 42);
    assert(bus.publish(TextEvent{"ignored"}) == 0);
}

void testPriorityAndStableOrder() {
    okapy::EventBus bus;
    std::vector<int> order;
    auto first = bus.subscribe<NumberEvent>(
        [&order](const NumberEvent&) { order.push_back(1); }, 0);
    auto second = bus.subscribe<NumberEvent>(
        [&order](const NumberEvent&) { order.push_back(2); }, 10);
    auto third = bus.subscribe<NumberEvent>(
        [&order](const NumberEvent&) { order.push_back(3); }, 10);

    bus.publish(NumberEvent{0});
    assert((order == std::vector<int>{2, 3, 1}));
}

void testHandleUnsubscribes() {
    okapy::EventBus bus;
    int calls = 0;
    {
        auto handle = bus.subscribe<NumberEvent>(
            [&calls](const NumberEvent&) { ++calls; });
        assert(handle.isActive());
        assert(bus.listenerCount() == 1);
    }
    assert(bus.listenerCount() == 0);
    assert(bus.publish(NumberEvent{1}) == 0);
    assert(calls == 0);
}

void testMoveAndExplicitReset() {
    okapy::EventBus bus;
    int calls = 0;
    auto original = bus.subscribe<NumberEvent>(
        [&calls](const NumberEvent&) { ++calls; });
    auto moved = std::move(original);
    assert(!original.isActive());
    assert(moved.isActive());
    moved.reset();
    assert(!moved.isActive());
    assert(bus.publish(NumberEvent{1}) == 0);
    assert(calls == 0);
}

void testClear() {
    okapy::EventBus bus;
    int calls = 0;
    auto number = bus.subscribe<NumberEvent>(
        [&calls](const NumberEvent&) { ++calls; });
    auto text = bus.subscribe<TextEvent>(
        [&calls](const TextEvent&) { ++calls; });

    bus.clear<NumberEvent>();
    assert(!number.isActive());
    assert(text.isActive());
    assert(bus.listenerCount() == 1);
    assert(bus.publish(NumberEvent{1}) == 0);
    assert(bus.publish(TextEvent{"ok"}) == 1);
    bus.clearAll();
    assert(bus.listenerCount() == 0);
}

void testSnapshotAllowsSubscriptionDuringDispatch() {
    okapy::EventBus bus;
    int calls = 0;
    okapy::ListenerHandle later;
    auto first = bus.subscribe<NumberEvent>(
        [&bus, &calls, &later](const NumberEvent&) {
            ++calls;
            if (!later.isActive()) {
                later = bus.subscribe<NumberEvent>(
                    [&calls](const NumberEvent&) { ++calls; });
            }
        });

    bus.publish(NumberEvent{1});
    assert(calls == 1);
    bus.publish(NumberEvent{2});
    assert(calls == 3);
}

} // namespace

int main() {
    testTypedDelivery();
    testPriorityAndStableOrder();
    testHandleUnsubscribes();
    testMoveAndExplicitReset();
    testClear();
    testSnapshotAllowsSubscriptionDuringDispatch();
    return 0;
}
