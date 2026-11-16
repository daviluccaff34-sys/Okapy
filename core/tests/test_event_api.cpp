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
    assert(host.event_subscribe && host.event_unsubscribe && host.event_publish);

    uint64_t token = 0;
    assert(host.event_subscribe(host.host_context, "mod.alpha", "player.died",
        &onEvent, nullptr, &token) == OKAPY_EVENT_PERMISSION_DENIED);

    assert(core.permissions().grant("mod.alpha", okapy::kPermissionEventsSubscribe));
    assert(core.permissions().grant("mod.alpha", okapy::kPermissionEventsPublish));
    assert(host.permission_check(host.host_context, "mod.alpha", "events.subscribe") == 1);
    assert(host.permission_check(host.host_context, "mod.beta", "events.subscribe") == 0);

    assert(host.event_subscribe(host.host_context, "mod.alpha", "player.died",
        &onEvent, nullptr, &token) == OKAPY_EVENT_OK);
    assert(token != 0);

    uint32_t invoked = 0;
    assert(host.event_publish(host.host_context, "mod.alpha", "player.died",
        R"({"attempt":3})", &invoked) == OKAPY_EVENT_OK);
    assert(invoked == 1 && callbackCount == 1);
    assert(receivedName == "player.died");
    assert(receivedPayload == R"({"attempt":3})");

    assert(host.event_unsubscribe(host.host_context, "mod.beta", token) ==
        OKAPY_EVENT_PERMISSION_DENIED);
    assert(host.event_unsubscribe(host.host_context, "mod.alpha", token) == OKAPY_EVENT_OK);
    assert(host.event_unsubscribe(host.host_context, "mod.alpha", token) == OKAPY_EVENT_NOT_FOUND);

    assert(host.event_publish(host.host_context, "mod.alpha", "level.completed",
        "{}", &invoked) == OKAPY_EVENT_OK);
    assert(invoked == 0);
    assert(host.event_subscribe(host.host_context, "mod.alpha", "",
        &onEvent, nullptr, &token) == OKAPY_EVENT_INVALID_ARGUMENT);
    assert(core.permissions().revoke("mod.alpha", okapy::kPermissionEventsPublish));
    assert(host.event_publish(host.host_context, "mod.alpha", "player.died",
        "{}", &invoked) == OKAPY_EVENT_PERMISSION_DENIED);
    return 0;
}
