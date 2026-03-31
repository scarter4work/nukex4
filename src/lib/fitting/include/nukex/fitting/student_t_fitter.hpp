#pragma once
#include "nukex/fitting/curve_fitter.hpp"
namespace nukex {
class StudentTFitter : public CurveFitter {
public:
    static constexpr float NU_GAUSSIAN_THRESHOLD = 30.0f;
    static constexpr double SIGMA_MIN = 1e-10;
    static constexpr double NU_MIN    = 2.0;
    static constexpr double NU_MAX    = 100.0;
    FitResult fit(const float* values, const float* weights,
                  int n, double robust_location, double robust_scale) override;
};
} // namespace nukex
