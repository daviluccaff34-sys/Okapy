#include <okapy/mod_abi.hpp>

#include <cstddef>

namespace okapy {

ModAbiValidation validateModAbi(
    OkapyModQueryV1 query,
    const OkapyModInfoV1& info,
    const OkapyModApiV1* api) noexcept {
    if (!query) return {ModAbiError::NullQuery};
    if (!api) return {ModAbiError::NullApi};

    constexpr auto infoMinimum =
        static_cast<std::uint32_t>(offsetof(OkapyModInfoV1, version) +
                                   sizeof(OkapyModInfoV1::version));
    constexpr auto apiMinimum =
        static_cast<std::uint32_t>(offsetof(OkapyModApiV1, shutdown) +
                                   sizeof(OkapyModApiV1::shutdown));

    if (info.struct_size < infoMinimum) return {ModAbiError::InvalidInfo};
    if (api->struct_size < apiMinimum) return {ModAbiError::InvalidApiSize};
    if (info.abi_version != OKAPY_MOD_ABI_VERSION_1 ||
        api->abi_version != OKAPY_MOD_ABI_VERSION_1) {
        return {ModAbiError::UnsupportedVersion};
    }
    if (!api->initialize) return {ModAbiError::MissingInitializer};
    if (!info.id || !*info.id || !info.name || !*info.name ||
        !info.version || !*info.version) {
        return {ModAbiError::MissingMetadata};
    }

    return {};
}

} // namespace okapy
