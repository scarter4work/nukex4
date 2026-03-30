# nukex4_core — header-only initially, source files added as features land.
add_library(nukex4_core INTERFACE)

target_include_directories(nukex4_core
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(nukex4_core INTERFACE cxx_std_17)
