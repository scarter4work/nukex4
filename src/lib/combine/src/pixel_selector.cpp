#include "nukex/combine/pixel_selector.hpp"
#include <cmath>
#include <algorithm>

namespace nukex {

float PixelSelector::sample_variance(float value, const FrameStats& fs,
                                      float welford_var) {
    if (fs.has_noise_keywords) {
        float rn = fs.read_noise;
        float g = fs.gain;
        if (g < 1e-10f) g = 1.0f;
        float shot_noise_var = std::max(0.0f, value / g);
        float read_noise_var = (rn * rn) / (g * g);
        return read_noise_var + shot_noise_var;
    }
    return welford_var;
}

void PixelSelector::select(const ZDistribution& dist,
                            const float* values, const float* weights, int n,
                            const FrameStats* frame_stats, const int* frame_indices,
                            float welford_var,
                            float& out_value, float& out_noise, float& out_snr) const {
    out_value = dist.true_signal_estimate;

    double weight_sum = 0.0;
    double variance_sum = 0.0;

    for (int i = 0; i < n; i++) {
        double w = static_cast<double>(weights[i]);
        int fi = frame_indices[i];
        float sigma2 = sample_variance(values[i], frame_stats[fi], welford_var);
        weight_sum += w;
        variance_sum += w * w * static_cast<double>(sigma2);
    }

    if (weight_sum > 1e-30) {
        out_noise = static_cast<float>(std::sqrt(variance_sum) / weight_sum);
    } else {
        out_noise = 0.0f;
    }

    if (out_noise > 1e-30f) {
        out_snr = std::clamp(out_value / out_noise, 0.0f, 9999.0f);
    } else {
        out_snr = 0.0f;
    }
}

} // namespace nukex
