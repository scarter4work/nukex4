#include "nukex/io/debayer.hpp"
#include <stdexcept>

namespace nukex {

Image DebayerEngine::debayer(const Image& raw, BayerPattern pattern) {
    if (raw.n_channels() != 1) {
        throw std::invalid_argument("DebayerEngine: input must be single-channel");
    }
    if (pattern == BayerPattern::NONE) {
        throw std::invalid_argument("DebayerEngine: BayerPattern::NONE is not debayerable");
    }

    Image rgb(raw.width(), raw.height(), 3);

    switch (pattern) {
        case BayerPattern::RGGB: debayer_rggb(raw, rgb); break;
        case BayerPattern::BGGR: debayer_bggr(raw, rgb); break;
        case BayerPattern::GRBG: debayer_grbg(raw, rgb); break;
        case BayerPattern::GBRG: debayer_gbrg(raw, rgb); break;
        default: break;
    }

    return rgb;
}

// Helper: clamp coordinates to image bounds
static inline int clamp_coord(int v, int max_val) {
    return (v < 0) ? 0 : (v >= max_val ? max_val - 1 : v);
}

// Helper: read raw pixel with boundary clamping
static inline float raw_at(const Image& raw, int x, int y) {
    int cx = clamp_coord(x, raw.width());
    int cy = clamp_coord(y, raw.height());
    return raw.at(cx, cy, 0);
}

/// Bilinear debayer for RGGB pattern.
///
/// RGGB 2x2 super-pixel layout:
///   (even_x, even_y) = R
///   (odd_x,  even_y) = G (on R row)
///   (even_x, odd_y)  = G (on B row)
///   (odd_x,  odd_y)  = B
///
/// For each pixel position, the color that IS present is read directly.
/// The two missing colors are interpolated from neighbors using bilinear averaging.
void DebayerEngine::debayer_rggb(const Image& raw, Image& rgb) {
    int w = raw.width();
    int h = raw.height();

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float r, g, b;
            bool even_x = (x % 2 == 0);
            bool even_y = (y % 2 == 0);

            if (even_x && even_y) {
                // R pixel: R is direct, G from 4 neighbors, B from 4 diagonal neighbors
                r = raw_at(raw, x, y);
                g = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y) +
                     raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.25f;
                b = (raw_at(raw, x-1, y-1) + raw_at(raw, x+1, y-1) +
                     raw_at(raw, x-1, y+1) + raw_at(raw, x+1, y+1)) * 0.25f;
            } else if (!even_x && even_y) {
                // G pixel on R row: G direct, R from left/right, B from top/bottom
                g = raw_at(raw, x, y);
                r = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y)) * 0.5f;
                b = (raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.5f;
            } else if (even_x && !even_y) {
                // G pixel on B row: G direct, B from left/right, R from top/bottom
                g = raw_at(raw, x, y);
                b = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y)) * 0.5f;
                r = (raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.5f;
            } else {
                // B pixel: B direct, G from 4 neighbors, R from 4 diagonals
                b = raw_at(raw, x, y);
                g = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y) +
                     raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.25f;
                r = (raw_at(raw, x-1, y-1) + raw_at(raw, x+1, y-1) +
                     raw_at(raw, x-1, y+1) + raw_at(raw, x+1, y+1)) * 0.25f;
            }

            rgb.at(x, y, 0) = r;
            rgb.at(x, y, 1) = g;
            rgb.at(x, y, 2) = b;
        }
    }
}

/// BGGR is RGGB rotated: swap R<->B positions.
void DebayerEngine::debayer_bggr(const Image& raw, Image& rgb) {
    int w = raw.width();
    int h = raw.height();

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float r, g, b;
            bool even_x = (x % 2 == 0);
            bool even_y = (y % 2 == 0);

            if (even_x && even_y) {
                // B pixel
                b = raw_at(raw, x, y);
                g = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y) +
                     raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.25f;
                r = (raw_at(raw, x-1, y-1) + raw_at(raw, x+1, y-1) +
                     raw_at(raw, x-1, y+1) + raw_at(raw, x+1, y+1)) * 0.25f;
            } else if (!even_x && even_y) {
                g = raw_at(raw, x, y);
                b = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y)) * 0.5f;
                r = (raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.5f;
            } else if (even_x && !even_y) {
                g = raw_at(raw, x, y);
                r = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y)) * 0.5f;
                b = (raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.5f;
            } else {
                r = raw_at(raw, x, y);
                g = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y) +
                     raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.25f;
                b = (raw_at(raw, x-1, y-1) + raw_at(raw, x+1, y-1) +
                     raw_at(raw, x-1, y+1) + raw_at(raw, x+1, y+1)) * 0.25f;
            }

            rgb.at(x, y, 0) = r;
            rgb.at(x, y, 1) = g;
            rgb.at(x, y, 2) = b;
        }
    }
}

/// GRBG: top-left is G on R row.
void DebayerEngine::debayer_grbg(const Image& raw, Image& rgb) {
    int w = raw.width();
    int h = raw.height();

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float r, g, b;
            bool even_x = (x % 2 == 0);
            bool even_y = (y % 2 == 0);

            if (even_x && even_y) {
                // G on R row
                g = raw_at(raw, x, y);
                r = (raw_at(raw, x+1, y) + raw_at(raw, x-1, y)) * 0.5f;
                b = (raw_at(raw, x, y+1) + raw_at(raw, x, y-1)) * 0.5f;
            } else if (!even_x && even_y) {
                // R pixel
                r = raw_at(raw, x, y);
                g = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y) +
                     raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.25f;
                b = (raw_at(raw, x-1, y-1) + raw_at(raw, x+1, y-1) +
                     raw_at(raw, x-1, y+1) + raw_at(raw, x+1, y+1)) * 0.25f;
            } else if (even_x && !even_y) {
                // B pixel
                b = raw_at(raw, x, y);
                g = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y) +
                     raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.25f;
                r = (raw_at(raw, x-1, y-1) + raw_at(raw, x+1, y-1) +
                     raw_at(raw, x-1, y+1) + raw_at(raw, x+1, y+1)) * 0.25f;
            } else {
                // G on B row
                g = raw_at(raw, x, y);
                b = (raw_at(raw, x+1, y) + raw_at(raw, x-1, y)) * 0.5f;
                r = (raw_at(raw, x, y+1) + raw_at(raw, x, y-1)) * 0.5f;
            }

            rgb.at(x, y, 0) = r;
            rgb.at(x, y, 1) = g;
            rgb.at(x, y, 2) = b;
        }
    }
}

/// GBRG: top-left is G on B row.
void DebayerEngine::debayer_gbrg(const Image& raw, Image& rgb) {
    int w = raw.width();
    int h = raw.height();

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float r, g, b;
            bool even_x = (x % 2 == 0);
            bool even_y = (y % 2 == 0);

            if (even_x && even_y) {
                // G on B row
                g = raw_at(raw, x, y);
                b = (raw_at(raw, x+1, y) + raw_at(raw, x-1, y)) * 0.5f;
                r = (raw_at(raw, x, y+1) + raw_at(raw, x, y-1)) * 0.5f;
            } else if (!even_x && even_y) {
                // B pixel
                b = raw_at(raw, x, y);
                g = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y) +
                     raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.25f;
                r = (raw_at(raw, x-1, y-1) + raw_at(raw, x+1, y-1) +
                     raw_at(raw, x-1, y+1) + raw_at(raw, x+1, y+1)) * 0.25f;
            } else if (even_x && !even_y) {
                // R pixel
                r = raw_at(raw, x, y);
                g = (raw_at(raw, x-1, y) + raw_at(raw, x+1, y) +
                     raw_at(raw, x, y-1) + raw_at(raw, x, y+1)) * 0.25f;
                b = (raw_at(raw, x-1, y-1) + raw_at(raw, x+1, y-1) +
                     raw_at(raw, x-1, y+1) + raw_at(raw, x+1, y+1)) * 0.25f;
            } else {
                // G on R row
                g = raw_at(raw, x, y);
                r = (raw_at(raw, x+1, y) + raw_at(raw, x-1, y)) * 0.5f;
                b = (raw_at(raw, x, y+1) + raw_at(raw, x, y-1)) * 0.5f;
            }

            rgb.at(x, y, 0) = r;
            rgb.at(x, y, 1) = g;
            rgb.at(x, y, 2) = b;
        }
    }
}

} // namespace nukex
