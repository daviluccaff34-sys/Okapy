#include <okapy/native_mod_loader.hpp>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <utility>

namespace okapy {

struct NativeModLoader::LoadedMod {
    NativeModule library;
    OkapyModApiV1 api{};
    OkapyHostApiV1 host{};
    NativeModLoadInfo info;
};

NativeModLoadResult NativeModLoader::load(
    std::string_view absolutePath,
    ModLifecycleManager& lifecycle,
    const OkapyHostApiV1& hostApi,
    NativeModLoadInfo* outInfo,
    std::string_view expectedModId) {
    constexpr auto hostMinimum =
        static_cast<std::uint32_t>(offsetof(OkapyHostApiV1, log) +
            sizeof(((OkapyHostApiV1*)nullptr)->log));
    if (hostApi.struct_size < hostMinimum ||
        hostApi.abi_version != OKAPY_MOD_ABI_VERSION_1) {
        return NativeModLoadResult::InvalidHostApi;
    }

    auto loaded = std::make_shared<LoadedMod>();
    const auto hostBytes = std::min<std::size_t>(
        hostApi.struct_size, sizeof(OkapyHostApiV1));
    std::memcpy(&loaded->host, &hostApi, hostBytes);
    loaded->host.struct_size = static_cast<std::uint32_t>(hostBytes);

    if (loaded->library.load(absolutePath) != NativeModuleResult::Loaded) {
        return NativeModLoadResult::LibraryLoadFailed;
    }

    void* rawSymbol = loaded->library.symbol(OKAPY_MOD_QUERY_SYMBOL);
    if (!rawSymbol) return NativeModLoadResult::MissingQuerySymbol;

    const auto query = reinterpret_cast<OkapyModQueryV1>(rawSymbol);
    OkapyModInfoV1 info{};
    info.struct_size = sizeof(info);
    info.abi_version = OKAPY_MOD_ABI_VERSION_1;

    // Calling the query function executes code from the loaded library.
    const auto* api = query(OKAPY_MOD_ABI_VERSION_1, &loaded->host, &info);
    const auto validation = validateModAbi(query, info, api);
    if (!validation.valid()) return NativeModLoadResult::InvalidAbi;

    loaded->api = *api;
    loaded->info.id = info.id;
    loaded->info.name = info.name;
    loaded->info.version = info.version;

    if (!ModRegistry::isValidId(loaded->info.id)) {
        return NativeModLoadResult::InvalidModId;
    }
    if (!expectedModId.empty() && loaded->info.id != expectedModId) {
        return NativeModLoadResult::ManifestIdMismatch;
    }
    if (m_loaded.find(loaded->info.id) != m_loaded.end()) {
        return NativeModLoadResult::DuplicateMod;
    }

    const auto id = loaded->info.id;
    const auto initialize = [loaded]() {
        return loaded->api.initialize(
            loaded->api.mod_context, &loaded->host) != 0;
    };
    const auto shutdown = [loaded]() {
        if (loaded->api.shutdown) {
            loaded->api.shutdown(loaded->api.mod_context);
        }
    };

    const auto registered = lifecycle.registerModule(id, initialize, shutdown);
    if (registered != LifecycleResult::Registered) {
        return NativeModLoadResult::LifecycleRegistrationFailed;
    }

    m_loaded.emplace(id, loaded);
    if (outInfo) *outInfo = loaded->info;
    return NativeModLoadResult::Registered;
}

bool NativeModLoader::contains(std::string_view id) const noexcept {
    return m_loaded.find(std::string(id)) != m_loaded.end();
}

std::vector<NativeModLoadInfo> NativeModLoader::list() const {
    std::vector<NativeModLoadInfo> result;
    result.reserve(m_loaded.size());
    for (const auto& item : m_loaded) result.push_back(item.second->info);
    std::sort(result.begin(), result.end(),
        [](const auto& left, const auto& right) { return left.id < right.id; });
    return result;
}

} // namespace okapy
