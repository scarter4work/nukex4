#pragma once

#include "nukex/io/image.hpp"
#include <string>

namespace nukex {

enum class StretchCategory { PRIMARY, SECONDARY, FINISHER };

/// Base class for all stretch operations.
///
/// Each op implements apply(Image&) for in-place image stretching and
/// optionally apply_scalar(float) for single-value computation (LUT, testing).
/// All ops work on float32 images normalized to [0, 1].
class StretchOp {
public:
    bool            enabled  = false;
    int             position = 0;
    std::string     name;
    StretchCategory category = StretchCategory::PRIMARY;

    virtual ~StretchOp() = default;

    /// Apply stretch to image in-place.
    virtual void apply(Image& img) const = 0;

    /// Apply to a single float value. Default: identity.
    virtual float apply_scalar(float x) const { return x; }
};

} // namespace nukex
