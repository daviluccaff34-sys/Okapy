#define OKAPY_MOD_BUILD
#include <okapy/mod_abi.h>

namespace {
int32_t initialize(void*, const OkapyHostApiV1* host) {
    if (host && host->log) host->log(host->host_context, 1, "fixture initialized");
    return 1;
}
void shutdown(void*) {}
const OkapyModApiV1 api = {
    sizeof(OkapyModApiV1),
    OKAPY_MOD_ABI_VERSION_1,
    nullptr,
    initialize,
    shutdown
};
}

extern "C" OKAPY_MOD_EXPORT const OkapyModApiV1* okapy_mod_query(
    uint32_t requestedAbi, const OkapyHostApiV1*, OkapyModInfoV1* outInfo) {
    if (requestedAbi != OKAPY_MOD_ABI_VERSION_1 || !outInfo) return nullptr;
    *outInfo = {
        sizeof(OkapyModInfoV1),
        OKAPY_MOD_ABI_VERSION_1,
        "fixture.test",
        "Okapy ABI Test Fixture",
        "1.0.0"
    };
    return &api;
}
