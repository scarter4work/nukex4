#include "nukex/alignment/star_matcher.hpp"
#include <cmath>
#include <unordered_map>

namespace nukex {

std::vector<std::pair<int, int>> StarMatcher::match(
    const StarCatalog& source,
    const StarCatalog& reference,
    float max_distance) {

    // For each source star, find the nearest reference star and record its distance.
    struct CandidateMatch {
        int source_idx;
        int ref_idx;
        float dist2;
    };

    std::vector<CandidateMatch> candidates;
    float max_dist2 = max_distance * max_distance;

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
            candidates.push_back({si, best_ri, best_dist2});
        }
    }

    // Deduplicate many-to-one matches: if multiple source stars matched the same
    // reference star, keep only the source with the smallest distance. This prevents
    // degenerate DLT matrices from duplicate reference assignments.
    std::unordered_map<int, size_t> best_for_ref;  // ref_idx -> index in candidates
    for (size_t i = 0; i < candidates.size(); i++) {
        int ri = candidates[i].ref_idx;
        auto it = best_for_ref.find(ri);
        if (it == best_for_ref.end()) {
            best_for_ref[ri] = i;
        } else if (candidates[i].dist2 < candidates[it->second].dist2) {
            it->second = i;
        }
    }

    std::vector<std::pair<int, int>> matches;
    matches.reserve(best_for_ref.size());
    for (const auto& [ri, idx] : best_for_ref) {
        matches.emplace_back(candidates[idx].source_idx, candidates[idx].ref_idx);
    }

    return matches;
}

} // namespace nukex
