#include <okapy/core.hpp>

#include <cassert>
#include <cstdint>
#include <string>

namespace {
int callbackCount = 0;
std::string receivedName;
std::string receivedPayload;

void onEvent(void*, const char* name, const char* payload) {
    ++callbackCount;
    receivedName = name ? name : "";
    receivedPayload = payload ? payload : "";
}
}

int main() {
    okapy::Core core;
    const auto host = core.hostApi();

    assert(host.event_subscribe != nullptr);
    assert(host.event_unsubscribe != nullptr);
    assert(host.event_publish != nullptr);

    uint64_t token = 0;
    assert(host.event_subscribe(host.host_context, "player.died",
        &onEvent, nullptr, &token) == OKAPY_EVENT_OK);
    assert(token != 0);

    uint32_t invoked = 0;
    assert(host.event_publish(host.host_context, "player.died",
        R"({"attempt":3})", &invoked) == OKAPY_EVENT_OK);
    assert(invoked == 1);
    assert(callbackCount == 1);
    assert(receivedName == "player.died");
    assert(receivedPayload == "{\\"attempt\\":3}");

    assert(host.event_publish(host.host_context, "level.completed",
        "{}", &invoked) == OKAPY_EVENT_OK);
    assert(invoked == 0);
    assert(callbackCount == 1);

    assert(host.event_unsubscribe(host.host_context, token) == OKAPY_EVENT_OK);
    assert(host.event_unsubscribe(host.host_context, token) == OKAPY_EVENT_NOT_FOUND);
    assert(host.event_publish(host.host_context, "player.died",
        "again", &invoked) == OKAPY_EVENT_OK);
    assert(invoked == 0);
    assert(callbackCount == 1);

    assert(host.event_subscribe(host.host_context, "", &onEvent,
        nullptr, &token) == OKAPY_EVENT_INVALID_ARGUMENT);
    return 0;
}
