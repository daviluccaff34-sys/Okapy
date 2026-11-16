#pragma once

#include <mutex>
#include <string>
#include <string_view>

namespace okapy {

enum class NativeModuleResult {
    Loaded,
    AlreadyLoaded,
    EmptyPath,
    RelativePath,
    LoadFailed
};

// Opens an explicitly supplied native library. Loading executes native code;
// callers must only load modules they trust. The module must outlive its symbols.
class NativeModule final {
public:
    NativeModule() = default;
    ~NativeModule();

    NativeModule(const NativeModule&) = delete;
    NativeModule& operator=(const NativeModule&) = delete;

    NativeModule(NativeModule&& other) noexcept;
    NativeModule& operator=(NativeModule&& other) noexcept;

    NativeModuleResult load(std::string_view absolutePath);
    bool unload() noexcept;

    [[nodiscard]] void* symbol(std::string_view name) const noexcept;
    [[nodiscard]] bool isLoaded() const noexcept;
    [[nodiscard]] std::string path() const;
    [[nodiscard]] std::string error() const;

private:
    mutable std::mutex m_mutex;
    void* m_handle = nullptr;
    std::string m_path;
    std::string m_error;
};

} // namespace okapy
