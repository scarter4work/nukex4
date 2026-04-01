#pragma once
#include "nukex/stretch/stretch_op.hpp"
namespace nukex {
class LogStretch : public StretchOp {
public:
    float alpha = 1000.0f;
    bool  luminance_only = false;
    LogStretch() { name = "Log"; category = StretchCategory::PRIMARY; }
    void apply(Image& img) const override;
    float apply_scalar(float x) const override;
};
} // namespace nukex
