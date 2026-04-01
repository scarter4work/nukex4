#include "nukex/stretch/log_stretch.hpp"
#include "nukex/stretch/stretch_utils.hpp"
#include <cmath>

namespace nukex {

float LogStretch::apply_scalar(float x) const {
    if (alpha < 1e-10f) return x;
    return std::log1p(alpha * x) / std::log1p(alpha);
}

void LogStretch::apply(Image& img) const {
    auto fn = [this](float x) { return apply_scalar(x); };

    if (luminance_only) {
        apply_luminance_only(img, fn);
    } else {
        apply_per_channel(img, fn);
    }
    clamp_image(img);
}

} // namespace nukex
