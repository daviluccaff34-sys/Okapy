#pragma once

#include <okapy/mod_abi.h>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace okapy {

enum class ConfigResult {
    Ok,
    NotFound,
    BufferTooSmall,
    InvalidModId,
    InvalidKey,
    ValueTooLarge,
    IoError,
    CorruptData
};

// Persistent UTF-8 string settings, stored in a separate file for each mod.
// The storage directory must be selected by the host application.
class ConfigStore final {
public:
    ConfigStore() = default;
    explicit ConfigStore(std::filesystem::path directory);

    ConfigStore(const ConfigStore&) = delete;
    ConfigStore& operator=(const ConfigStore&) = delete;

    bool setStorageDirectory(std::filesystem::path directory);
    [[nodiscard]] std::filesystem::path storageDirectory() const;

    ConfigResult set(std::string_view modId, std::string_view key,
                     std::string_view value);
    ConfigResult get(std::string_view modId, std::string_view key,
                     std::string& value) const;
    ConfigResult erase(std::string_view modId, std::string_view key);
    ConfigResult clearMod(std::string_view modId);

    // Returns a Host API table whose config callbacks use this store.
    // The store must outlive every native mod using the returned table.
    [[nodiscard]] OkapyHostApiV1 hostApi() noexcept;

private:
    using Values = std::unordered_map<std::string, std::string>;

    static bool validKey(std::string_view key) noexcept;
    static int32_t configGetCallback(void* context, const char* modId,
        const char* key, char* buffer, uint32_t capacity,
        uint32_t* requiredSize) noexcept;
    static int32_t configSetCallback(void* context, const char* modId,
        const char* key, const char* value) noexcept;

    ConfigResult loadLocked(const std::string& modId) const;
    ConfigResult saveLocked(const std::string& modId) const;
    std::filesystem::path filePath(const std::string& modId) const;

    mutable std::mutex m_mutex;
    std::filesystem::path m_directory;
    mutable std::unordered_map<std::string, Values> m_values;
    mutable std::unordered_map<std::string, ConfigResult> m_loadResults;
};

} // namespace okapy
