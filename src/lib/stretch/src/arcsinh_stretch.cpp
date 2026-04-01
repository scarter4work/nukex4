#include "nukex/stretch/arcsinh_stretch.hpp"
#include "nukex/stretch/stretch_utils.hpp"
#include <cmath>

namespace nukex {

float ArcSinhStretch::apply_scalar(float x) const {
    if (alpha < 1e-10f) return x;  // Identity when alpha ≈ 0
    return std::asinh(alpha * x) / std::asinh(alpha);
}

void ArcSinhStretch::apply(Image& img) const {
    auto fn = [this](float x) { return apply_scalar(x); };

    if (luminance_only) {
        apply_luminance_only(img, fn);
    } else {
        apply_per_channel(img, fn);
    }
    clamp_image(img);
}

} // namespace nukex
