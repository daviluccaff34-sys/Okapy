add_library(okapy_core STATIC
    src/core.cpp
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
