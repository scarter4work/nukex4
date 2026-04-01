#include "test_data_loader.hpp"
#include "nukex/io/fits_reader.hpp"
#include "nukex/io/debayer.hpp"
#include "nukex/core/channel_config.hpp"
#include <filesystem>

namespace nukex { namespace test_util {

const std::string& m16_data_dir() {
    static const std::string dir = "/home/scarter4work/projects/processing/M16/";
    return dir;
}

Image load_m16_test_frame() {
    std::string dir = m16_data_dir();
    if (!std::filesystem::exists(dir)) return {};

    // Find first .fit file
    std::string path;
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        auto ext = entry.path().extension().string();
        if (ext == ".fit" || ext == ".fits") {
            path = entry.path().string();
            break;
        }
    }
    if (path.empty()) return {};

    auto result = FITSReader::read(path);
    if (!result.success) return {};

    // Debayer if Bayer pattern detected
    if (!result.metadata.bayer_pattern.empty() &&
        result.metadata.bayer_pattern != "NONE") {
        // Parse bayer pattern
        BayerPattern bp = BayerPattern::RGGB;  // Default for ZWO cameras
        if (result.metadata.bayer_pattern == "BGGR") bp = BayerPattern::BGGR;
        else if (result.metadata.bayer_pattern == "GRBG") bp = BayerPattern::GRBG;
        else if (result.metadata.bayer_pattern == "GBRG") bp = BayerPattern::GBRG;

        return DebayerEngine::debayer(result.image, bp);
    }

    return std::move(result.image);
}

}} // namespace nukex::test_util
