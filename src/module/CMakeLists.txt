# NukeX v4 PixInsight Module
# Builds NukeX-pxm.so (Linux), NukeX-pxm.dylib (macOS), NukeX-pxm.dll (Windows)

set(PCLDIR "$ENV{HOME}/PCL" CACHE PATH "PixInsight Class Library directory")

if(NOT EXISTS "${PCLDIR}/include/pcl/MetaModule.h")
    message(STATUS "PCL not found at ${PCLDIR} -- skipping module build")
    return()
endif()

message(STATUS "PCL found at ${PCLDIR} -- building NukeX module")

set(MODULE_SOURCES
    NukeXModule.cpp
    NukeXProcess.cpp
    NukeXInstance.cpp
    NukeXInterface.cpp
    NukeXParameters.cpp
)

add_library(NukeX-pxm SHARED ${MODULE_SOURCES})

# PCL include directories
target_include_directories(NukeX-pxm PRIVATE
    ${PCLDIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# PCL compile definitions
target_compile_definitions(NukeX-pxm PRIVATE
    __PCL_LINUX
    __PCL_BUILDING_MODULE
    _REENTRANT
)

# PCL compile flags
target_compile_options(NukeX-pxm PRIVATE
    -fPIC
    -fvisibility=hidden
    -fvisibility-inlines-hidden
)

# Link NukeX v4 libraries
target_link_libraries(NukeX-pxm PRIVATE
    nukex4_stacker
    nukex4_gpu
    nukex4_stretch
    nukex4_combine
    nukex4_fitting
    nukex4_classify
    nukex4_alignment
    nukex4_io
    nukex4_core
)

# Link PCL static libraries
target_link_directories(NukeX-pxm PRIVATE ${PCLDIR}/lib/x64)
target_link_libraries(NukeX-pxm PRIVATE
    PCL-pxi
    lz4-pxi
    zstd-pxi
    zlib-pxi
    RFC6234-pxi
    lcms-pxi
    cminpack-pxi
)

# Shared library flags
set_target_properties(NukeX-pxm PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "NukeX-pxm"
    LINK_FLAGS "-shared -Wl,-z,defs"
)

target_compile_features(NukeX-pxm PRIVATE cxx_std_17)
