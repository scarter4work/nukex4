#include "nukex/combine/output_assembler.hpp"

namespace nukex {

Image OutputAssembler::assemble_quality_map(const Cube& cube) {
    Image quality(cube.width, cube.height, 4);

    for (int y = 0; y < cube.height; y++) {
        for (int x = 0; x < cube.width; x++) {
            const auto& v = cube.at(x, y);
            int primary_ch = 0;
            const auto& dist = v.distribution[primary_ch];

            quality.at(x, y, 0) = dist.true_signal_estimate;
            quality.at(x, y, 1) = dist.signal_uncertainty;
            quality.at(x, y, 2) = dist.confidence;
            quality.at(x, y, 3) = static_cast<float>(
                static_cast<uint8_t>(dist.shape));
        }
    }

    return quality;
}

} // namespace nukex
