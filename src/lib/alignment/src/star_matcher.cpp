#include "nukex/alignment/star_matcher.hpp"
#include <cmath>

namespace nukex {

std::vector<std::pair<int, int>> StarMatcher::match(
    const StarCatalog& source,
    const StarCatalog& reference,
    float max_distance) {

    std::vector<std::pair<int, int>> matches;
    float max_dist2 = max_distance * max_distance;

    // For each source star, find the nearest reference star
    for (int si = 0; si < source.size(); si++) {
        float best_dist2 = max_dist2;
        int best_ri = -1;

        for (int ri = 0; ri < reference.size(); ri++) {
            float dx = source.stars[si].x - reference.stars[ri].x;
            float dy = source.stars[si].y - reference.stars[ri].y;
            float d2 = dx * dx + dy * dy;

            if (d2 < best_dist2) {
                best_dist2 = d2;
                best_ri = ri;
            }
        }

        if (best_ri >= 0) {
            matches.emplace_back(si, best_ri);
        }
    }

    return matches;
}

} // namespace nukex
