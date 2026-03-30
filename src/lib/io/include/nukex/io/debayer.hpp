#pragma once

#include "nukex/io/image.hpp"
#include "nukex/core/channel_config.hpp"

namespace nukex {

/// Debayer (demosaic) a raw Bayer image into RGB channels.
///
/// Input: single-channel float32 image with Bayer pattern data.
/// Output: 3-channel float32 image (R, G, B).
///
/// Uses bilinear interpolation. This is intentionally simple — for stacking,
/// the debayered output feeds into alignment and accumulation, not direct display.
/// The distribution fitting on the accumulated Z-arrays is what produces the
/// final pixel values, so debayer quality at this stage is not critical.
class DebayerEngine {
public:
    /// Debayer a raw Bayer image to RGB.
    /// Returns a 3-channel image. Input must be single-channel.
    static Image debayer(const Image& raw, BayerPattern pattern);

private:
    /// Bilinear interpolation for RGGB pattern.
    static void debayer_rggb(const Image& raw, Image& rgb);
    /// Bilinear interpolation for BGGR pattern.
    static void debayer_bggr(const Image& raw, Image& rgb);
    /// Bilinear interpolation for GRBG pattern.
    static void debayer_grbg(const Image& raw, Image& rgb);
    /// Bilinear interpolation for GBRG pattern.
    static void debayer_gbrg(const Image& raw, Image& rgb);
};

} // namespace nukex
