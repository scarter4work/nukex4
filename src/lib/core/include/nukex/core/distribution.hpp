#pragma once

#include <cstdint>

namespace nukex {

enum class DistributionShape : uint8_t {
    GAUSSIAN      = 0,
    BIMODAL       = 1,
    SKEWED_LOW    = 2,
    SKEWED_HIGH   = 3,
    SPIKE_OUTLIER = 4,
    UNIFORM       = 5,
    UNKNOWN       = 6
};

inline const char* distribution_shape_name(DistributionShape shape) {
    switch (shape) {
        case DistributionShape::GAUSSIAN:      return "GAUSSIAN";
        case DistributionShape::BIMODAL:       return "BIMODAL";
        case DistributionShape::SKEWED_LOW:    return "SKEWED_LOW";
        case DistributionShape::SKEWED_HIGH:   return "SKEWED_HIGH";
        case DistributionShape::SPIKE_OUTLIER: return "SPIKE_OUTLIER";
        case DistributionShape::UNIFORM:       return "UNIFORM";
        case DistributionShape::UNKNOWN:       return "UNKNOWN";
    }
    return "UNKNOWN";
}

struct GaussianParams {
    float mu        = 0.0f;
    float sigma     = 0.0f;
    float amplitude = 0.0f;
};

struct BimodalParams {
    GaussianParams comp1;
    GaussianParams comp2;
    float          mixing_ratio = 0.0f;
};

struct SkewNormalParams {
    float mu    = 0.0f;
    float sigma = 0.0f;
    float alpha = 0.0f;
};

struct ZDistribution {
    DistributionShape shape = DistributionShape::UNKNOWN;

    union {
        GaussianParams   gaussian;
        BimodalParams    bimodal;
        SkewNormalParams skew_normal;
        GaussianParams   spike_main;
    } params = {};

    float   spike_value       = 0.0f;
    uint8_t spike_frame_index = 0;

    float r_squared = 0.0f;
    float aic       = 0.0f;
    float bic       = 0.0f;

    float true_signal_estimate = 0.0f;
    float signal_uncertainty   = 0.0f;
    float confidence           = 0.0f;

    float kde_mode      = 0.0f;
    float kde_bandwidth = 0.0f;
    bool  used_nonparametric = false;
};

} // namespace nukex
