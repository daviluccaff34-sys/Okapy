#include <okapy/mod_abi.hpp>

#include <cassert>
#include <cstddef>

namespace {

int32_t initialize(void*, const OkapyHostApiV1*) { return 1; }
void shutdown(void*) {}

const OkapyModApiV1 validApi = {
    sizeof(OkapyModApiV1), OKAPY_MOD_ABI_VERSION_1, nullptr,
    initialize, shutdown
};
const OkapyModInfoV1 validInfo = {
    sizeof(OkapyModInfoV1), OKAPY_MOD_ABI_VERSION_1,
    "sample.mod", "Sample Mod", "1.2.3"
};
const OkapyModApiV1* query(
    uint32_t, const OkapyHostApiV1*, OkapyModInfoV1*) {
    return &validApi;
}

void testValidAbi() {
    assert(okapy::validateModAbi(query, validInfo, &validApi).valid());
}

void testMissingParts() {
    assert(okapy::validateModAbi(nullptr, validInfo, &validApi).error ==
           okapy::ModAbiError::NullQuery);
    assert(okapy::validateModAbi(query, validInfo, nullptr).error ==
           okapy::ModAbiError::NullApi);
}

void testInvalidMetadataAndVersion() {
    auto info = validInfo;
    info.id = nullptr;
    assert(okapy::validateModAbi(query, info, &validApi).error ==
           okapy::ModAbiError::MissingMetadata);

    info = validInfo;
    info.abi_version = 99;
    assert(okapy::validateModAbi(query, info, &validApi).error ==
           okapy::ModAbiError::UnsupportedVersion);
}

void testInvalidApi() {
    auto api = validApi;
    api.initialize = nullptr;
    assert(okapy::validateModAbi(query, validInfo, &api).error ==
           okapy::ModAbiError::MissingInitializer);

    api = validApi;
    api.struct_size = 1;
    assert(okapy::validateModAbi(query, validInfo, &api).error ==
           okapy::ModAbiError::InvalidApiSize);
}

} // namespace

int main() {
    testValidAbi();
    testMissingParts();
    testInvalidMetadataAndVersion();
    testInvalidApi();
    return 0;
}
