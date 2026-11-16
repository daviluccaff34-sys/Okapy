#include <okapy/core.hpp>

#include <cassert>
#include <string>

int main() {
    okapy::PermissionManager permissions;
    assert(!permissions.has("mod.alpha", okapy::kPermissionConfigRead));
    assert(permissions.grant("mod.alpha", okapy::kPermissionConfigRead));
    assert(!permissions.grant("mod.alpha", okapy::kPermissionConfigRead));
    assert(permissions.has("mod.alpha", okapy::kPermissionConfigRead));
    assert(!permissions.has("mod.beta", okapy::kPermissionConfigRead));
    assert(!permissions.grant("../bad", okapy::kPermissionConfigRead));
    assert(!permissions.grant("mod.alpha", "filesystem.root"));

    const auto granted = permissions.list("mod.alpha");
    assert(granted.size() == 1 && granted.front() == "config.read");
    assert(permissions.revoke("mod.alpha", okapy::kPermissionConfigRead));
    assert(!permissions.has("mod.alpha", okapy::kPermissionConfigRead));

    assert(permissions.grant("mod.alpha", okapy::kPermissionConfigWrite));
    permissions.clearMod("mod.alpha");
    assert(permissions.list("mod.alpha").empty());

    okapy::Core core;
    const auto host = core.hostApi();
    assert(host.permission_check != nullptr);
    assert(host.permission_check(host.host_context, "mod.alpha", "config.read") == 0);
    assert(host.permission_check(host.host_context, nullptr, "config.read") == -1);
    return 0;
}
