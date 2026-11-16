#include <okapy/native_module.hpp>

#include <cassert>
#include <string>

#ifndef OKAPY_TEST_MODULE_PATH
#error OKAPY_TEST_MODULE_PATH must point to the test shared library
#endif

int main() {
    okapy::NativeModule module;

    assert(!module.isLoaded());
    assert(module.load("") == okapy::NativeModuleResult::EmptyPath);
    assert(module.load("relative/library.so") == okapy::NativeModuleResult::RelativePath);
    assert(!module.error().empty());

    const std::string libraryPath = OKAPY_TEST_MODULE_PATH;
    assert(module.load(libraryPath) == okapy::NativeModuleResult::Loaded);
    assert(module.isLoaded());
    assert(module.path() == libraryPath);
    assert(module.load(libraryPath) == okapy::NativeModuleResult::AlreadyLoaded);

    using ValueFunction = int (*)();
    auto value = reinterpret_cast<ValueFunction>(
        module.symbol("okapy_test_module_value"));
    assert(value != nullptr);
    assert(value() == 42);
    assert(module.symbol("missing_symbol") == nullptr);

    assert(module.unload());
    assert(!module.isLoaded());
    assert(module.path().empty());
    assert(module.symbol("okapy_test_module_value") == nullptr);
    assert(module.unload());

    assert(module.load("/this/path/should/not/exist/okapy_missing_module.so")
           == okapy::NativeModuleResult::LoadFailed);
    assert(!module.error().empty());
    return 0;
}
