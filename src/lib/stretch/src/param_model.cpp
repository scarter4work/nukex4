#include "nukex/stretch/param_model.hpp"

#include <algorithm>
#include <cmath>

namespace nukex {

ParamModel::ParamModel(std::string stretch_name)
    : stretch_name_(std::move(stretch_name)) {}

void ParamModel::add_param(const std::string& param_name, ParamCoefficients coeffs) {
    per_param_.emplace(param_name, std::move(coeffs));
}

namespace {

bool row_finite(const std::array<double, 29>& row) {
    for (double v : row) if (!std::isfinite(v)) return false;
    return true;
}

double predict_scalar(const ParamCoefficients& c, const std::array<double, 29>& row) {
    double sum = c.intercept;
    const auto n = std::min<std::size_t>(row.size(), c.coefficients.size());
    for (std::size_t i = 0; i < n; ++i) {
        const double std_ = (c.feature_std[i] > 1e-12) ? c.feature_std[i] : 1.0;
        const double z    = (row[i] - c.feature_mean[i]) / std_;
        sum += c.coefficients[i] * z;
    }
    return sum;
}

} // namespace

bool ParamModel::predict_and_apply(const ImageStats& stats, StretchOp& op) const {
    const auto row = stats.to_feature_row();
    if (!row_finite(row)) return false;

    const auto bounds = op.param_bounds();
    if (bounds.empty()) return false;

    bool applied_any = false;
    for (const auto& [pname, coeffs] : per_param_) {
        auto it = bounds.find(pname);
        if (it == bounds.end()) continue;

        double v = predict_scalar(coeffs, row);
        if (!std::isfinite(v)) continue;

        const float lo = it->second.first;
        const float hi = it->second.second;
        if (v < lo) v = lo;
        if (v > hi) v = hi;

        if (op.set_param(pname, static_cast<float>(v))) {
            applied_any = true;
        }
    }
    return applied_any;
}

} // namespace nukex
