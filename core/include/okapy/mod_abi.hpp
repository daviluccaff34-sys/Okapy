#pragma once

#include <okapy/mod_abi.h>

namespace okapy {

enum class ModAbiError {
    None,
    NullQuery,
    NullApi,
    InvalidInfo,
    InvalidApiSize,
    UnsupportedVersion,
    MissingInitializer,
    MissingMetadata
};

struct ModAbiValidation {
    ModAbiError error = ModAbiError::None;
    [[nodiscard]] bool valid() const noexcept {
        return error == ModAbiError::None;
    }
};

// Validates the returned structures without invoking mod code. The query
// function itself must only be called after the host has decided to trust/load
// the native library; even querying a library executes its code.
[[nodiscard]] ModAbiValidation validateModAbi(
    OkapyModQueryV1 query,
    const OkapyModInfoV1& info,
    const OkapyModApiV1* api) noexcept;

} // namespace okapy
