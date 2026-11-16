#include <okapy/native_module.hpp>

#include <filesystem>
#include <utility>

#if defined(_WIN32)
#  define NOMINMAX
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

namespace okapy {

NativeModule::~NativeModule() {
    unload();
}

NativeModule::NativeModule(NativeModule&& other) noexcept {
    std::lock_guard<std::mutex> lock(other.m_mutex);
    m_handle = other.m_handle;
    m_path = std::move(other.m_path);
    m_error = std::move(other.m_error);
    other.m_handle = nullptr;
    other.m_path.clear();
    other.m_error.clear();
}

NativeModule& NativeModule::operator=(NativeModule&& other) noexcept {
    if (this == &other) return *this;

    unload();
    std::scoped_lock lock(m_mutex, other.m_mutex);
    m_handle = other.m_handle;
    m_path = std::move(other.m_path);
    m_error = std::move(other.m_error);
    other.m_handle = nullptr;
    other.m_path.clear();
    other.m_error.clear();
    return *this;
}

NativeModuleResult NativeModule::load(std::string_view absolutePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_handle) return NativeModuleResult::AlreadyLoaded;
    if (absolutePath.empty()) {
        m_error = "The library path is empty.";
        return NativeModuleResult::EmptyPath;
    }

    const std::filesystem::path requestedPath{std::string(absolutePath)};
    if (!requestedPath.is_absolute()) {
        m_error = "A native library must be loaded using an absolute path.";
        return NativeModuleResult::RelativePath;
    }

#if defined(_WIN32)
    const auto handle = LoadLibraryW(requestedPath.wstring().c_str());
    if (!handle) {
        m_error = "LoadLibraryW failed with Windows error " +
                  std::to_string(GetLastError()) + ".";
        return NativeModuleResult::LoadFailed;
    }
    m_handle = reinterpret_cast<void*>(handle);
#else
    dlerror();
    m_handle = dlopen(requestedPath.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!m_handle) {
        const char* message = dlerror();
        m_error = message ? message : "dlopen failed.";
        return NativeModuleResult::LoadFailed;
    }
#endif

    m_path = requestedPath.string();
    m_error.clear();
    return NativeModuleResult::Loaded;
}

bool NativeModule::unload() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_handle) return true;

#if defined(_WIN32)
    const bool success = FreeLibrary(reinterpret_cast<HMODULE>(m_handle)) != 0;
    if (!success) {
        m_error = "FreeLibrary failed with Windows error " +
                  std::to_string(GetLastError()) + ".";
        return false;
    }
#else
    if (dlclose(m_handle) != 0) {
        const char* message = dlerror();
        m_error = message ? message : "dlclose failed.";
        return false;
    }
#endif

    m_handle = nullptr;
    m_path.clear();
    m_error.clear();
    return true;
}

void* NativeModule::symbol(std::string_view name) const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_handle || name.empty()) return nullptr;

    const std::string symbolName(name);
#if defined(_WIN32)
    const auto address = GetProcAddress(
        reinterpret_cast<HMODULE>(m_handle), symbolName.c_str());
    return reinterpret_cast<void*>(address);
#else
    dlerror();
    return dlsym(m_handle, symbolName.c_str());
#endif
}

bool NativeModule::isLoaded() const noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_handle != nullptr;
}

std::string NativeModule::path() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_path;
}

std::string NativeModule::error() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_error;
}

} // namespace okapy
