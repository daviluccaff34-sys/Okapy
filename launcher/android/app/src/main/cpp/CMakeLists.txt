cmake_minimum_required(VERSION 3.22.1)

project(okapy_android LANGUAGES CXX)

# Resolve the repository's existing Core instead of maintaining an Android copy.
get_filename_component(OKAPY_REPOSITORY_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../../"
    ABSOLUTE
)

set(BUILD_TESTING OFF CACHE BOOL "Disable host Core tests in Android builds" FORCE)
add_subdirectory("${OKAPY_REPOSITORY_ROOT}/core" "${CMAKE_CURRENT_BINARY_DIR}/okapy-core")

add_library(okapy_jni SHARED
    native-lib.cpp
)

target_compile_features(okapy_jni PRIVATE cxx_std_17)
target_link_libraries(okapy_jni
    PRIVATE
        Okapy::Core
        log
)

set_target_properties(okapy_jni PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
