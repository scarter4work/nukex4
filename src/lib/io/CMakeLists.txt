find_package(PkgConfig REQUIRED)
pkg_check_modules(CFITSIO REQUIRED cfitsio)

add_library(nukex4_io STATIC
    src/image.cpp
)

target_include_directories(nukex4_io
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${CFITSIO_INCLUDE_DIRS}
)

target_link_libraries(nukex4_io
    PUBLIC nukex4_core
    PRIVATE ${CFITSIO_LIBRARIES}
)

target_compile_features(nukex4_io PUBLIC cxx_std_17)
