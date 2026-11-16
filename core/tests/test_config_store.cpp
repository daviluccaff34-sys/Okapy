#include <okapy/config_store.hpp>

#include <cassert>
#include <filesystem>
#include <string>

int main() {
    const auto directory = std::filesystem::temp_directory_path() /
        "okapy-config-store-test";
    std::error_code ec;
    std::filesystem::remove_all(directory, ec);

    {
        okapy::ConfigStore store(directory);
        assert(store.set("sample.one", "theme", "dark") == okapy::ConfigResult::Ok);
        assert(store.set("sample.two", "theme", "light") == okapy::ConfigResult::Ok);
        assert(store.set("sample.one", "welcome", "hello\nworld") == okapy::ConfigResult::Ok);

        std::string value;
        assert(store.get("sample.one", "theme", value) == okapy::ConfigResult::Ok);
        assert(value == "dark");
        assert(store.get("sample.two", "theme", value) == okapy::ConfigResult::Ok);
        assert(value == "light");
        assert(store.get("sample.one", "missing", value) == okapy::ConfigResult::NotFound);
        assert(store.set("../escape", "x", "y") == okapy::ConfigResult::InvalidModId);
    }

    {
        okapy::ConfigStore reopened(directory);
        std::string value;
        assert(reopened.get("sample.one", "welcome", value) == okapy::ConfigResult::Ok);
        assert(value == "hello\nworld");
        assert(reopened.erase("sample.one", "theme") == okapy::ConfigResult::Ok);
        assert(reopened.get("sample.one", "theme", value) == okapy::ConfigResult::NotFound);
    }

    {
        okapy::ConfigStore store(directory);
        auto host = store.hostApi();
        assert(host.config_get != nullptr && host.config_set != nullptr);
        assert(host.config_set(host.host_context, "sample.callback", "enabled", "yes") ==
               OKAPY_CONFIG_OK);
        char buffer[8]{};
        uint32_t required = 0;
        assert(host.config_get(host.host_context, "sample.callback", "enabled",
                               buffer, sizeof(buffer), &required) == OKAPY_CONFIG_OK);
        assert(std::string(buffer) == "yes");
        assert(required == 4);
    }

    std::filesystem::remove_all(directory, ec);
    return 0;
}
