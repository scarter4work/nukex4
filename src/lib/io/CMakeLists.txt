# Use static CFITSIO built without curl to avoid OpenSSL conflicts with PI's bundled libs.
# Falls back to system pkg-config cfitsio if static build not found.
set(CFITSIO_STATIC_DIR "/tmp/cfitsio-static" CACHE PATH "Static CFITSIO install prefix")

if(EXISTS "${CFITSIO_STATIC_DIR}/lib/libcfitsio.a")
    set(CFITSIO_INCLUDE_DIRS "${CFITSIO_STATIC_DIR}/include")
    set(CFITSIO_LIBRARIES "${CFITSIO_STATIC_DIR}/lib/libcfitsio.a" z m)
    message(STATUS "Using static CFITSIO from ${CFITSIO_STATIC_DIR} (no curl/ssl)")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CFITSIO REQUIRED cfitsio)
    message(STATUS "Using system CFITSIO (dynamic, may have curl/ssl deps)")
endif()

add_library(nukex4_io STATIC
    src/image.cpp
    src/fits_reader.cpp
    src/debayer.cpp
    src/flat_calibration.cpp
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
