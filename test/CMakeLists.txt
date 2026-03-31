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
nukex_add_test(test_image         unit/io/test_image.cpp           nukex4_io)
nukex_add_test(test_fits_reader   unit/io/test_fits_reader.cpp     nukex4_io)
nukex_add_test(test_debayer       unit/io/test_debayer.cpp         nukex4_io)
nukex_add_test(test_welford       unit/core/test_welford.cpp       nukex4_core)
nukex_add_test(test_histogram     unit/core/test_histogram.cpp     nukex4_core)
nukex_add_test(test_reservoir     unit/core/test_reservoir.cpp     nukex4_core)
nukex_add_test(test_distribution  unit/core/test_distribution.cpp  nukex4_core)
nukex_add_test(test_channel       unit/core/test_channel_config.cpp nukex4_core)
nukex_add_test(test_voxel         unit/core/test_voxel.cpp          nukex4_core)
nukex_add_test(test_cube          unit/core/test_cube.cpp           nukex4_core)
nukex_add_test(test_flat          unit/io/test_flat_calibration.cpp nukex4_io)
nukex_add_test(test_star_detector unit/alignment/test_star_detector.cpp nukex4_alignment)
nukex_add_test(test_homography     unit/alignment/test_homography.cpp     nukex4_alignment)
nukex_add_test(test_frame_aligner unit/alignment/test_frame_aligner.cpp nukex4_alignment)
nukex_add_test(test_robust_stats  unit/fitting/test_robust_stats.cpp   nukex4_fitting)
nukex_add_test(test_student_t  unit/fitting/test_student_t_fitter.cpp  nukex4_fitting)
nukex_add_test(test_gmm        unit/fitting/test_gmm_fitter.cpp       nukex4_fitting)
nukex_add_test(test_contamination unit/fitting/test_contamination_fitter.cpp nukex4_fitting)
nukex_add_test(test_kde          unit/fitting/test_kde_fitter.cpp         nukex4_fitting)
nukex_add_test(test_model_selector unit/fitting/test_model_selector.cpp nukex4_fitting)
nukex_add_test(test_weight_computer unit/classify/test_weight_computer.cpp nukex4_classify)
nukex_add_test(test_pixel_selector unit/combine/test_pixel_selector.cpp nukex4_combine)
nukex_add_test(test_spatial_context unit/combine/test_spatial_context.cpp nukex4_combine)
nukex_add_test(test_output_assembler unit/combine/test_output_assembler.cpp nukex4_combine)
