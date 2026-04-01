#pragma once

#include "nukex/io/image.hpp"
#include <string>

namespace nukex { namespace test_util {

/// Write an Image to a 16-bit PNG file for visual evaluation.
///
/// For float32 [0,1] images, maps to [0, 65535] uint16.
/// Multi-channel images are written as RGB (first 3 channels).
/// Single-channel images are written as grayscale.
///
/// Applies a simple arcsinh preview stretch if apply_stretch is true,
/// to make linear astronomical data visible.
///
/// Returns true on success.
bool write_png(const std::string& filepath, const nukex::Image& img,
               bool apply_stretch = true, float stretch_alpha = 500.0f);

}} // namespace nukex::test_util
