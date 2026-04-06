#include "catch_amalgamated.hpp"
#include "nukex/stacker/stacking_engine.hpp"
#include "nukex/io/image.hpp"
#include <filesystem>

using namespace nukex;

TEST_CASE("StackingEngine: empty input produces empty result", "[engine]") {
    StackingEngine::Config cfg;
    cfg.cache_dir = "/tmp";
    StackingEngine engine(cfg);

    auto result = engine.execute({}, {});
    REQUIRE(result.n_frames_processed == 0);
    REQUIRE(result.stacked.empty());
    REQUIRE(result.noise_map.empty());
    REQUIRE(result.quality_map.empty());
    REQUIRE(result.n_frames_failed_alignment == 0);
}

// Integration test with real FITS data (skip if not available)
TEST_CASE("StackingEngine: M16 integration test", "[engine][integration][!mayfail]") {
    std::string data_dir = "/home/scarter4work/projects/processing/M16/";
    if (!std::filesystem::exists(data_dir)) {
        SKIP("M16 test data not available at " + data_dir);
    }

    // Find first 5 FITS files
    std::vector<std::string> lights;
    for (const auto& entry : std::filesystem::directory_iterator(data_dir)) {
        auto ext = entry.path().extension().string();
        if (ext == ".fits" || ext == ".fit") {
            lights.push_back(entry.path().string());
            if (lights.size() >= 5) break;
        }
    }
    if (lights.size() < 3) {
        SKIP("Not enough FITS files in " + data_dir);
    }

    StackingEngine::Config cfg;
    cfg.cache_dir = "/tmp";
    StackingEngine engine(cfg);

    auto result = engine.execute(lights, {});
    REQUIRE(result.n_frames_processed >= 3);
    REQUIRE(!result.stacked.empty());
    REQUIRE(result.stacked.width() > 0);
    REQUIRE(result.stacked.height() > 0);
    REQUIRE(!result.noise_map.empty());
    REQUIRE(!result.quality_map.empty());
    REQUIRE(result.quality_map.n_channels() == 4);
}
