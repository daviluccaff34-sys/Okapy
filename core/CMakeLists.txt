add_library(okapy_core STATIC
    src/core.cpp
    src/mod_registry.cpp
)

add_library(Okapy::Core ALIAS okapy_core)

target_include_directories(okapy_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(okapy_core PUBLIC cxx_std_17)
set_target_properties(okapy_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_EXTENSIONS OFF
)

include(CTest)
if(BUILD_TESTING)
    add_executable(okapy_core_tests tests/test_mod_registry.cpp)
    target_link_libraries(okapy_core_tests PRIVATE Okapy::Core)
    add_test(NAME okapy_core_tests COMMAND okapy_core_tests)
endif()
