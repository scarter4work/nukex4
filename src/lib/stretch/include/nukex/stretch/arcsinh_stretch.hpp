#pragma once
#include "nukex/stretch/stretch_op.hpp"
namespace nukex {
class ArcSinhStretch : public StretchOp {
public:
    float alpha = 500.0f;
    bool  luminance_only = true;
    ArcSinhStretch() { name = "ArcSinh"; category = StretchCategory::PRIMARY; }
    void apply(Image& img) const override;
    float apply_scalar(float x) const override;
};
} // namespace nukex
