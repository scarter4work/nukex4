#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "png_writer.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace nukex { namespace test_util {

bool write_png(const std::string& filepath, const nukex::Image& img,
               bool apply_stretch, float stretch_alpha) {
    if (img.empty()) return false;

    int w = img.width();
    int h = img.height();
    int ch = std::min(img.n_channels(), 3);  // Max 3 channels for PNG RGB

    // Convert to 16-bit interleaved RGB (or grayscale)
    std::vector<uint16_t> pixels(w * h * ch);

    float norm = (stretch_alpha > 0.0f) ? std::asinh(stretch_alpha) : 1.0f;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            for (int c = 0; c < ch; c++) {
                float val = img.at(x, y, c);

                // Apply arcsinh preview stretch if requested
                if (apply_stretch && stretch_alpha > 0.0f) {
                    val = std::asinh(stretch_alpha * val) / norm;
                }

                val = std::clamp(val, 0.0f, 1.0f);
                uint16_t u16 = static_cast<uint16_t>(val * 65535.0f + 0.5f);

                // stb_image_write expects big-endian for 16-bit PNG
                uint16_t be = (u16 >> 8) | (u16 << 8);
                pixels[(y * w + x) * ch + c] = be;
            }
        }
    }

    int stride = w * ch * 2;  // 2 bytes per component
    return stbi_write_png(filepath.c_str(), w, h, ch, pixels.data(), stride) != 0;
}

}} // namespace nukex::test_util
