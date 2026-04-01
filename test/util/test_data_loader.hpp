#pragma once

#include "nukex/io/image.hpp"
#include <string>

namespace nukex { namespace test_util {

/// Load the first M16 FITS frame, debayer it, and return as a 3-channel RGB Image.
/// Returns an empty Image if the file is not available.
///
/// The M16 data is HaO3 narrowband on a Bayer sensor (ZWO ASI2400MC Pro).
/// After debayer, it's a 3-channel float32 Image in [0, 1].
Image load_m16_test_frame();

/// Path to the M16 test data directory.
const std::string& m16_data_dir();

}} // namespace nukex::test_util
