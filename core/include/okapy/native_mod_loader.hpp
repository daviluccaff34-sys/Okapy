#pragma once

#include <okapy/mod_abi.hpp>
#include <okapy/mod_lifecycle.hpp>
#include <okapy/native_module.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace okapy {

enum class NativeModLoadResult {
    Registered,
    InvalidHostApi,
    LibraryLoadFailed,
    EntrypointNotFound,
    MissingQuerySymbol,
    QueryRejected,
    InvalidAbi,
    InvalidModId,
    DuplicateMod,
    LifecycleRegistrationFailed,
    ManifestIdMismatch
};

struct NativeModLoadInfo {
    std::string id;
    std::string name;
    std::string version;
};

class NativeModLoader final {
public:
    NativeModLoader() = default;
    ~NativeModLoader() = default;

    NativeModLoader(const NativeModLoader&) = delete;
    NativeModLoader& operator=(const NativeModLoader&) = delete;

    // Loading/querying native libraries executes their code. Only use trusted
    // absolute paths. The lifecycle manager must outlive this loader.
    NativeModLoadResult load(
        std::string_view absolutePath,
        ModLifecycleManager& lifecycle,
        const OkapyHostApiV1& hostApi,
        NativeModLoadInfo* outInfo = nullptr,
        std::string_view expectedModId = {});

    [[nodiscard]] bool contains(std::string_view id) const noexcept;
    [[nodiscard]] std::vector<NativeModLoadInfo> list() const;

private:
    struct LoadedMod;
    std::unordered_map<std::string, std::shared_ptr<LoadedMod>> m_loaded;
};

} // namespace okapy
