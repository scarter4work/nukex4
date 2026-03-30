#include "catch_amalgamated.hpp"
#include "nukex/alignment/homography.hpp"
#include "nukex/alignment/star_matcher.hpp"
#include "nukex/alignment/star_detector.hpp"
#include "nukex/io/image.hpp"
#include <cmath>

using namespace nukex;

TEST_CASE("HomographyComputer: identity from identical catalogs", "[homography]") {
    // Two identical star catalogs should produce identity homography
    StarCatalog cat;
    for (int i = 0; i < 20; i++) {
        Star s;
        s.x = 50.0f + static_cast<float>(i % 5) * 80.0f;
        s.y = 50.0f + static_cast<float>(i / 5) * 80.0f;
        s.flux = 100.0f - static_cast<float>(i);
        cat.stars.push_back(s);
    }

    auto matches = StarMatcher::match(cat, cat, 5.0f);
    REQUIRE(matches.size() >= 8);

    auto result = HomographyComputer::compute(cat, cat, matches);
    REQUIRE(result.match.success == true);
    REQUIRE(result.alignment_failed == false);
    REQUIRE(result.H.is_identity(0.01f) == true);
    REQUIRE(result.match.rms_error < 0.1f);
}

TEST_CASE("HomographyComputer: translation recovery", "[homography]") {
    // Source catalog shifted by (10, 5)
    StarCatalog ref, src;
    float dx = 10.0f, dy = 5.0f;

    for (int i = 0; i < 20; i++) {
        Star s;
        s.x = 100.0f + static_cast<float>(i % 5) * 60.0f;
        s.y = 100.0f + static_cast<float>(i / 5) * 60.0f;
        s.flux = 100.0f;
        ref.stars.push_back(s);

        Star s2 = s;
        s2.x += dx;
        s2.y += dy;
        src.stars.push_back(s2);
    }

    // Match with large enough distance to find shifted pairs
    auto matches = StarMatcher::match(src, ref, 20.0f);
    REQUIRE(matches.size() >= 8);

    auto result = HomographyComputer::compute(src, ref, matches);
    REQUIRE(result.match.success == true);

    // Verify: transform a source point should give the reference point
    auto [tx, ty] = result.H.transform(ref.stars[0].x + dx, ref.stars[0].y + dy);
    REQUIRE(tx == Catch::Approx(ref.stars[0].x).margin(0.5f));
    REQUIRE(ty == Catch::Approx(ref.stars[0].y).margin(0.5f));
}

TEST_CASE("HomographyComputer: too few matches fails gracefully", "[homography]") {
    StarCatalog src, ref;
    for (int i = 0; i < 3; i++) {
        Star s;
        s.x = static_cast<float>(i) * 100.0f;
        s.y = 100.0f;
        src.stars.push_back(s);
        ref.stars.push_back(s);
    }

    auto matches = StarMatcher::match(src, ref, 5.0f);
    auto result = HomographyComputer::compute(src, ref, matches);

    REQUIRE(result.alignment_failed == true);
    REQUIRE(result.weight_penalty == Catch::Approx(0.5f));
}

TEST_CASE("HomographyComputer: warp with identity preserves image", "[homography]") {
    Image img(20, 20, 1);
    for (int y = 0; y < 20; y++)
        for (int x = 0; x < 20; x++)
            img.at(x, y, 0) = static_cast<float>(x + y * 20) / 400.0f;

    auto H = HomographyMatrix::identity();
    Image warped = HomographyComputer::warp(img, H, 20, 20);

    // Interior pixels should be preserved exactly
    for (int y = 1; y < 19; y++) {
        for (int x = 1; x < 19; x++) {
            REQUIRE(warped.at(x, y, 0) == Catch::Approx(img.at(x, y, 0)).margin(0.01f));
        }
    }
}

TEST_CASE("HomographyComputer: meridian flip correction", "[homography]") {
    // 180-degree rotation
    HomographyMatrix H;
    H(0,0) = -1; H(0,1) = 0; H(0,2) = 99;
    H(1,0) = 0;  H(1,1) = -1; H(1,2) = 79;
    H(2,0) = 0;  H(2,1) = 0;  H(2,2) = 1;

    REQUIRE(H.is_meridian_flip() == true);

    auto corrected = HomographyComputer::correct_meridian_flip(H, 100, 80);
    // After correction, should be approximately identity
    REQUIRE(corrected.is_identity(1.0f) == true);
}

TEST_CASE("StarMatcher: identical catalogs match all", "[star_matcher]") {
    StarCatalog cat;
    for (int i = 0; i < 10; i++) {
        Star s;
        s.x = static_cast<float>(i * 50);
        s.y = static_cast<float>(i * 30);
        cat.stars.push_back(s);
    }

    auto matches = StarMatcher::match(cat, cat, 5.0f);
    REQUIRE(matches.size() == 10);

    // Each star should match itself
    for (const auto& [si, ri] : matches) {
        REQUIRE(si == ri);
    }
}
