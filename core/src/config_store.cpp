#include <okapy/config_store.hpp>
#include <okapy/mod_registry.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <limits>
#include <system_error>
#include <utility>
#include <vector>

namespace okapy {
namespace {
constexpr std::array<char, 4> kMagic{{'O', 'K', 'C', 'F'}};
constexpr uint32_t kFormatVersion = 1;
constexpr uint32_t kMaxEntries = 4096;
constexpr uint32_t kMaxKeyBytes = 256;
constexpr uint32_t kMaxValueBytes = 64 * 1024;
constexpr uint64_t kMaxFileBytes = 4 * 1024 * 1024;

bool writeU32(std::ostream& out, uint32_t value) {
    const char bytes[4] = {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
        static_cast<char>((value >> 16) & 0xff),
        static_cast<char>((value >> 24) & 0xff)
    };
    out.write(bytes, sizeof(bytes));
    return static_cast<bool>(out);
}

bool readU32(std::istream& in, uint32_t& value) {
    unsigned char bytes[4]{};
    in.read(reinterpret_cast<char*>(bytes), sizeof(bytes));
    if (!in) return false;
    value = static_cast<uint32_t>(bytes[0]) |
        (static_cast<uint32_t>(bytes[1]) << 8) |
        (static_cast<uint32_t>(bytes[2]) << 16) |
        (static_cast<uint32_t>(bytes[3]) << 24);
    return true;
}

bool writeString(std::ostream& out, const std::string& value) {
    out.write(value.data(), static_cast<std::streamsize>(value.size()));
    return static_cast<bool>(out);
}
}

ConfigStore::ConfigStore(std::filesystem::path directory)
    : m_directory(std::move(directory)) {}

bool ConfigStore::setStorageDirectory(std::filesystem::path directory) {
    if (directory.empty()) return false;
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_values.empty()) return false;
    m_directory = std::move(directory);
    m_loadResults.clear();
    return true;
}

std::filesystem::path ConfigStore::storageDirectory() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_directory;
}

bool ConfigStore::validKey(std::string_view key) noexcept {
    if (key.empty() || key.size() > kMaxKeyBytes) return false;
    for (const unsigned char ch : key) {
        if (ch < 0x20 || ch == 0x7f) return false;
    }
    return true;
}

std::filesystem::path ConfigStore::filePath(const std::string& modId) const {
    return m_directory / (modId + ".okcfg");
}

ConfigResult ConfigStore::loadLocked(const std::string& modId) const {
    const auto known = m_loadResults.find(modId);
    if (known != m_loadResults.end()) return known->second;
    if (m_directory.empty()) return ConfigResult::IoError;

    std::error_code ec;
    const auto path = filePath(modId);
    if (!std::filesystem::exists(path, ec)) {
        if (ec) return ConfigResult::IoError;
        m_values.emplace(modId, Values{});
        m_loadResults.emplace(modId, ConfigResult::Ok);
        return ConfigResult::Ok;
    }

    const auto fileSize = std::filesystem::file_size(path, ec);
    if (ec || fileSize > kMaxFileBytes) {
        m_loadResults.emplace(modId, ConfigResult::CorruptData);
        return ConfigResult::CorruptData;
    }

    std::ifstream in(path, std::ios::binary);
    if (!in) return ConfigResult::IoError;
    std::array<char, 4> magic{};
    in.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    uint32_t version = 0, count = 0;
    if (!in || magic != kMagic || !readU32(in, version) ||
        !readU32(in, count) || version != kFormatVersion ||
        count > kMaxEntries) {
        m_loadResults.emplace(modId, ConfigResult::CorruptData);
        return ConfigResult::CorruptData;
    }

    Values loaded;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t keySize = 0, valueSize = 0;
        if (!readU32(in, keySize) || !readU32(in, valueSize) ||
            keySize == 0 || keySize > kMaxKeyBytes ||
            valueSize > kMaxValueBytes) {
            m_loadResults.emplace(modId, ConfigResult::CorruptData);
            return ConfigResult::CorruptData;
        }
        std::string key(keySize, '\0'), value(valueSize, '\0');
        in.read(key.data(), static_cast<std::streamsize>(keySize));
        in.read(value.data(), static_cast<std::streamsize>(valueSize));
        if (!in || !validKey(key) || loaded.find(key) != loaded.end()) {
            m_loadResults.emplace(modId, ConfigResult::CorruptData);
            return ConfigResult::CorruptData;
        }
        loaded.emplace(std::move(key), std::move(value));
    }
    if (in.peek() != std::char_traits<char>::eof()) {
        m_loadResults.emplace(modId, ConfigResult::CorruptData);
        return ConfigResult::CorruptData;
    }
    m_values.emplace(modId, std::move(loaded));
    m_loadResults.emplace(modId, ConfigResult::Ok);
    return ConfigResult::Ok;
}

ConfigResult ConfigStore::saveLocked(const std::string& modId) const {
    if (m_directory.empty()) return ConfigResult::IoError;
    const auto found = m_values.find(modId);
    if (found == m_values.end()) return ConfigResult::IoError;
    if (found->second.size() > kMaxEntries) return ConfigResult::ValueTooLarge;

    std::error_code ec;
    std::filesystem::create_directories(m_directory, ec);
    if (ec) return ConfigResult::IoError;

    const auto destination = filePath(modId);
    auto temporary = destination;
    temporary += ".tmp";
    std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
    if (!out) return ConfigResult::IoError;
    out.write(kMagic.data(), static_cast<std::streamsize>(kMagic.size()));
    if (!writeU32(out, kFormatVersion) ||
        !writeU32(out, static_cast<uint32_t>(found->second.size()))) {
        out.close();
        std::filesystem::remove(temporary, ec);
        return ConfigResult::IoError;
    }

    std::vector<std::string> keys;
    keys.reserve(found->second.size());
    for (const auto& entry : found->second) keys.push_back(entry.first);
    std::sort(keys.begin(), keys.end());
    for (const auto& key : keys) {
        const auto& value = found->second.at(key);
        if (key.size() > kMaxKeyBytes || value.size() > kMaxValueBytes ||
            !writeU32(out, static_cast<uint32_t>(key.size())) ||
            !writeU32(out, static_cast<uint32_t>(value.size())) ||
            !writeString(out, key) || !writeString(out, value)) {
            out.close();
            std::filesystem::remove(temporary, ec);
            return ConfigResult::ValueTooLarge;
        }
    }
    out.flush();
    if (!out) {
        out.close();
        std::filesystem::remove(temporary, ec);
        return ConfigResult::IoError;
    }
    out.close();

    std::filesystem::rename(temporary, destination, ec);
    if (ec) {
        // Windows does not replace an existing destination with rename().
        std::error_code removeError;
        std::filesystem::remove(destination, removeError);
        ec.clear();
        std::filesystem::rename(temporary, destination, ec);
    }
    if (ec) {
        std::filesystem::remove(temporary, ec);
        return ConfigResult::IoError;
    }
    return ConfigResult::Ok;
}

ConfigResult ConfigStore::set(std::string_view modId, std::string_view key,
                              std::string_view value) {
    if (!ModRegistry::isValidId(modId)) return ConfigResult::InvalidModId;
    if (!validKey(key)) return ConfigResult::InvalidKey;
    if (value.size() > kMaxValueBytes) return ConfigResult::ValueTooLarge;
    std::lock_guard<std::mutex> lock(m_mutex);
    const std::string id(modId);
    const auto loaded = loadLocked(id);
    if (loaded != ConfigResult::Ok) return loaded;
    auto& values = m_values[id];
    const std::string ownedKey(key);
    const auto existing = values.find(ownedKey);
    if (existing == values.end() && values.size() >= kMaxEntries)
        return ConfigResult::ValueTooLarge;
    const bool hadValue = existing != values.end();
    const std::string oldValue = hadValue ? existing->second : std::string{};
    values[ownedKey] = std::string(value);
    const auto saved = saveLocked(id);
    if (saved != ConfigResult::Ok) {
        if (hadValue) values[ownedKey] = oldValue;
        else values.erase(ownedKey);
    }
    return saved;
}

ConfigResult ConfigStore::get(std::string_view modId, std::string_view key,
                              std::string& value) const {
    if (!ModRegistry::isValidId(modId)) return ConfigResult::InvalidModId;
    if (!validKey(key)) return ConfigResult::InvalidKey;
    std::lock_guard<std::mutex> lock(m_mutex);
    const std::string id(modId);
    const auto loaded = loadLocked(id);
    if (loaded != ConfigResult::Ok) return loaded;
    const auto& values = m_values.at(id);
    const auto found = values.find(std::string(key));
    if (found == values.end()) return ConfigResult::NotFound;
    value = found->second;
    return ConfigResult::Ok;
}

ConfigResult ConfigStore::erase(std::string_view modId, std::string_view key) {
    if (!ModRegistry::isValidId(modId)) return ConfigResult::InvalidModId;
    if (!validKey(key)) return ConfigResult::InvalidKey;
    std::lock_guard<std::mutex> lock(m_mutex);
    const std::string id(modId);
    const auto loaded = loadLocked(id);
    if (loaded != ConfigResult::Ok) return loaded;
    auto& values = m_values[id];
    const auto found = values.find(std::string(key));
    if (found == values.end()) return ConfigResult::NotFound;
    const auto oldValue = found->second;
    values.erase(found);
    const auto saved = saveLocked(id);
    if (saved != ConfigResult::Ok) values.emplace(std::string(key), oldValue);
    return saved;
}

ConfigResult ConfigStore::clearMod(std::string_view modId) {
    if (!ModRegistry::isValidId(modId)) return ConfigResult::InvalidModId;
    std::lock_guard<std::mutex> lock(m_mutex);
    const std::string id(modId);
    const auto loaded = loadLocked(id);
    if (loaded != ConfigResult::Ok) return loaded;
    auto oldValues = m_values[id];
    m_values[id].clear();
    const auto saved = saveLocked(id);
    if (saved != ConfigResult::Ok) m_values[id] = std::move(oldValues);
    return saved;
}

int32_t ConfigStore::configGetCallback(void* context, const char* modId,
    const char* key, char* buffer, uint32_t capacity,
    uint32_t* requiredSize) noexcept {
    if (!context || !modId || !key || !requiredSize) return OKAPY_CONFIG_ERROR;
    try {
        std::string value;
        const auto result = static_cast<ConfigStore*>(context)->get(modId, key, value);
        if (result == ConfigResult::NotFound) return OKAPY_CONFIG_NOT_FOUND;
        if (result != ConfigResult::Ok) return OKAPY_CONFIG_ERROR;
        if (value.size() >= std::numeric_limits<uint32_t>::max())
            return OKAPY_CONFIG_ERROR;
        *requiredSize = static_cast<uint32_t>(value.size() + 1);
        if (!buffer || capacity < *requiredSize) return OKAPY_CONFIG_BUFFER_TOO_SMALL;
        std::copy(value.begin(), value.end(), buffer);
        buffer[value.size()] = '\0';
        return OKAPY_CONFIG_OK;
    } catch (...) {
        return OKAPY_CONFIG_ERROR;
    }
}

int32_t ConfigStore::configSetCallback(void* context, const char* modId,
    const char* key, const char* value) noexcept {
    if (!context || !modId || !key || !value) return OKAPY_CONFIG_ERROR;
    try {
        const auto result = static_cast<ConfigStore*>(context)->set(modId, key, value);
        return result == ConfigResult::Ok ? OKAPY_CONFIG_OK : OKAPY_CONFIG_ERROR;
    } catch (...) {
        return OKAPY_CONFIG_ERROR;
    }
}

OkapyHostApiV1 ConfigStore::hostApi() noexcept {
    OkapyHostApiV1 api{};
    api.struct_size = sizeof(api);
    api.abi_version = OKAPY_MOD_ABI_VERSION_1;
    api.host_context = this;
    api.config_get = &ConfigStore::configGetCallback;
    api.config_set = &ConfigStore::configSetCallback;
    return api;
}

} // namespace okapy
