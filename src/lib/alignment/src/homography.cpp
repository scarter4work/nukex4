#include "nukex/alignment/types.hpp"
#include <cmath>

namespace nukex {

bool HomographyMatrix::is_identity(float tolerance) const {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            float expected = (r == c) ? 1.0f : 0.0f;
            if (std::abs(H[r][c] - expected) > tolerance) return false;
        }
    }
    return true;
}

float HomographyMatrix::rotation_degrees() const {
    return std::atan2(H[1][0], H[0][0]) * 180.0f / 3.14159265358979f;
}

bool HomographyMatrix::is_meridian_flip(float angle_tolerance_deg) const {
    float angle = std::abs(rotation_degrees());
    return std::abs(angle - 180.0f) < angle_tolerance_deg;
}

HomographyMatrix HomographyMatrix::identity() {
    HomographyMatrix m;
    m.H = {{{1,0,0}, {0,1,0}, {0,0,1}}};
    return m;
}

} // namespace nukex
