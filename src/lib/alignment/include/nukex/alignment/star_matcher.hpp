#pragma once

#include "nukex/alignment/types.hpp"

namespace nukex {

/// Match stars between two catalogs using nearest-neighbor search.
class StarMatcher {
public:
    struct Config {
        float max_distance = 5.0f;     // maximum match distance in pixels
        int   min_matches  = 8;        // minimum matches for valid alignment
    };

    /// Find nearest-neighbor matches between source and reference catalogs.
    /// Returns pairs of (source_idx, ref_idx).
    static std::vector<std::pair<int, int>> match(
        const StarCatalog& source,
        const StarCatalog& reference,
        float max_distance);
};

} // namespace nukex
