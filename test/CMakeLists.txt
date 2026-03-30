# Catch2 amalgamated — compile the source file once as a static library
add_library(catch2_main STATIC
    "${CMAKE_SOURCE_DIR}/third_party/catch2/catch_amalgamated.cpp"
)
target_include_directories(catch2_main PUBLIC
    "${CMAKE_SOURCE_DIR}/third_party/catch2"
)

# Helper function to add a test executable
function(nukex_add_test test_name test_source)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE catch2_main ${ARGN})
    target_include_directories(${test_name} PRIVATE
        "${CMAKE_SOURCE_DIR}/third_party/catch2"
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Core library tests — each added when implemented (Tasks 4-10)
nukex_add_test(test_welford       unit/core/test_welford.cpp       nukex4_core)
