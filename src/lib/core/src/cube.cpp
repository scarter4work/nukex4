#include "nukex/core/cube.hpp"

namespace nukex {

Cube::Cube(int w, int h, const ChannelConfig& config)
    : width(w)
    , height(h)
    , channel_config(config)
    , n_frames_loaded(0)
    , voxels_(static_cast<size_t>(w) * static_cast<size_t>(h))
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            SubcubeVoxel& v = voxels_[y * width + x];
            v.n_channels = config.n_channels;
            for (int ch = 0; ch < config.n_channels; ch++) {
                uint64_t seed = 14695981039346656037ULL;
                seed ^= static_cast<uint64_t>(x);
                seed *= 1099511628211ULL;
                seed ^= static_cast<uint64_t>(y);
                seed *= 1099511628211ULL;
                seed ^= static_cast<uint64_t>(ch);
                seed *= 1099511628211ULL;
                v.reservoir[ch].seed(seed);
            }
        }
    }
}

} // namespace nukex
