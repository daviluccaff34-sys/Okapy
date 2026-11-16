add_library(okapy_core STATIC
    src/core.cpp
    src/mod_registry.cpp
    src/dependency_manager.cpp
    src/hook_manager.cpp
    src/patch_manager.cpp
    src/mod_loader.cpp
    src/native_module.cpp
    src/mod_lifecycle.cpp
    src/mod_abi.cpp
    src/native_mod_loader.cpp
)

add_library(Okapy::Core ALIAS okapy_core)

target_include_directories(okapy_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(okapy_core PUBLIC cxx_std_17)
target_link_libraries(okapy_core PRIVATE ${CMAKE_DL_LIBS})
set_target_properties(okapy_core PROPERTIES POSITION_INDEPENDENT_CODE ON CXX_EXTENSIONS OFF)

include(CTest)
if(BUILD_TESTING)
    add_executable(okapy_core_tests tests/test_mod_registry.cpp)
    target_link_libraries(okapy_core_tests PRIVATE Okapy::Core)
    add_test(NAME okapy_core_tests COMMAND okapy_core_tests)
    add_executable(okapy_dependency_tests tests/test_dependency_manager.cpp)
    target_link_libraries(okapy_dependency_tests PRIVATE Okapy::Core)
    add_test(NAME okapy_dependency_tests COMMAND okapy_dependency_tests)
    add_executable(okapy_event_tests tests/test_event_system.cpp)
    target_link_libraries(okapy_event_tests PRIVATE Okapy::Core)
    add_test(NAME okapy_event_tests COMMAND okapy_event_tests)
    add_executable(okapy_hook_tests tests/test_hook_manager.cpp)
    target_link_libraries(okapy_hook_tests PRIVATE Okapy::Core)
    add_test(NAME okapy_hook_tests COMMAND okapy_hook_tests)
    add_executable(okapy_patch_tests tests/test_patch_manager.cpp)
    target_link_libraries(okapy_patch_tests PRIVATE Okapy::Core)
    add_test(NAME okapy_patch_tests COMMAND okapy_patch_tests)
    add_executable(okapy_mod_loader_tests tests/test_mod_loader.cpp)
    target_link_libraries(okapy_mod_loader_tests PRIVATE Okapy::Core)
    add_test(NAME okapy_mod_loader_tests COMMAND okapy_mod_loader_tests)
    add_library(okapy_test_native_module MODULE tests/native_test_module.cpp)
    set_target_properties(okapy_test_native_module PROPERTIES CXX_VISIBILITY_PRESET default VISIBILITY_INLINES_HIDDEN OFF)
    add_executable(okapy_native_module_tests tests/test_native_module.cpp)
    target_link_libraries(okapy_native_module_tests PRIVATE Okapy::Core)
    target_compile_definitions(okapy_native_module_tests PRIVATE OKAPY_TEST_MODULE_PATH="$<TARGET_FILE:okapy_test_native_module>")
    add_dependencies(okapy_native_module_tests okapy_test_native_module)
    add_test(NAME okapy_native_module_tests COMMAND okapy_native_module_tests)
    add_executable(okapy_mod_lifecycle_tests tests/test_mod_lifecycle.cpp)
    target_link_libraries(okapy_mod_lifecycle_tests PRIVATE Okapy::Core)
    add_test(NAME okapy_mod_lifecycle_tests COMMAND okapy_mod_lifecycle_tests)
    add_executable(okapy_mod_abi_tests tests/test_mod_abi.cpp)
    target_link_libraries(okapy_mod_abi_tests PRIVATE Okapy::Core)
    add_test(NAME okapy_mod_abi_tests COMMAND okapy_mod_abi_tests)
    add_library(okapy_abi_test_mod MODULE tests/abi_test_mod.cpp)
    target_include_directories(okapy_abi_test_mod PRIVATE include)
    set_target_properties(okapy_abi_test_mod PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN OFF)
    add_executable(okapy_native_mod_loader_tests tests/test_native_mod_loader.cpp)
    target_link_libraries(okapy_native_mod_loader_tests PRIVATE Okapy::Core)
    target_compile_definitions(okapy_native_mod_loader_tests PRIVATE OKAPY_ABI_TEST_MODULE_PATH="$<TARGET_FILE:okapy_abi_test_mod>")
    add_dependencies(okapy_native_mod_loader_tests okapy_abi_test_mod)
    add_test(NAME okapy_native_mod_loader_tests COMMAND okapy_native_mod_loader_tests)
endif()