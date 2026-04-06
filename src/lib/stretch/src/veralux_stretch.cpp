#include "nukex/stretch/veralux_stretch.hpp"
#include "nukex/stretch/stretch_utils.hpp"
#include <cmath>
#include <algorithm>

namespace nukex {

float VeraLuxStretch::apply_scalar(float x) const {
    // Core arcsinh curve: [arcsinh(D*(x-SP)+b) - arcsinh(b)] / [arcsinh(D*(1-SP)+b) - arcsinh(b)]
    // With SP = 0 (shadow point at black):
    //   f(x) = [arcsinh(D*x + b) - arcsinh(b)] / [arcsinh(D + b) - arcsinh(b)]
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;

    float D = std::pow(10.0f, log_D);
    float b = protect_b;

    float num = std::asinh(D * x + b) - std::asinh(b);
    float den = std::asinh(D + b) - std::asinh(b);

    if (den < 1e-10f) return x;  // Degenerate case
    return std::clamp(num / den, 0.0f, 1.0f);
}

void VeraLuxStretch::apply(Image& img) const {
    int n = img.width() * img.height();
    int nch = img.n_channels();

    float D = std::pow(10.0f, log_D);
    float b = protect_b;
    float den = std::asinh(D + b) - std::asinh(b);
    if (den < 1e-10f) return;

    if (nch < 3) {
        // Single channel: apply scalar stretch
        float* data = img.channel_data(0);
        for (int i = 0; i < n; i++)
            data[i] = apply_scalar(data[i]);
        clamp_image(img);
        return;
    }

    // Multi-channel: color vector preservation with convergence-to-white
    float* R = img.channel_data(0);
    float* G = img.channel_data(1);
    float* B = img.channel_data(2);

    constexpr float eps = 1e-9f;

    for (int i = 0; i < n; i++) {
        float r = R[i], g = G[i], bv = B[i];

        // Sensor-weighted luminance
        float L = w_R * r + w_G * g + w_B * bv;
        if (L < eps) continue;

        // Chromaticity ratios (color direction)
        float r_ratio = r / (L + eps);
        float g_ratio = g / (L + eps);
        float b_ratio = bv / (L + eps);

        // Stretch luminance
        float L_stretched = (std::asinh(D * L + b) - std::asinh(b)) / den;
        L_stretched = std::clamp(L_stretched, 0.0f, 1.0f);

        // Convergence factor: bright pixels transition toward white
        // k approaches 1 for bright pixels, 0 for faint pixels
        float k = std::pow(L_stretched, convergence_power);

        // Reconstruct: blend between original color ratio and white (1,1,1)
        R[i] = std::clamp(L_stretched * (r_ratio * (1.0f - k) + k), 0.0f, 1.0f);
        G[i] = std::clamp(L_stretched * (g_ratio * (1.0f - k) + k), 0.0f, 1.0f);
        B[i] = std::clamp(L_stretched * (b_ratio * (1.0f - k) + k), 0.0f, 1.0f);
    }
}

} // namespace nukex
