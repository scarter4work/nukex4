#include "catch_amalgamated.hpp"
#include "nukex/core/distribution.hpp"

using namespace nukex;

TEST_CASE("DistributionShape enum values are correct", "[distribution]") {
    REQUIRE(static_cast<uint8_t>(DistributionShape::GAUSSIAN) == 0);
    REQUIRE(static_cast<uint8_t>(DistributionShape::BIMODAL) == 1);
    REQUIRE(static_cast<uint8_t>(DistributionShape::SKEWED_LOW) == 2);
    REQUIRE(static_cast<uint8_t>(DistributionShape::SKEWED_HIGH) == 3);
    REQUIRE(static_cast<uint8_t>(DistributionShape::SPIKE_OUTLIER) == 4);
    REQUIRE(static_cast<uint8_t>(DistributionShape::UNIFORM) == 5);
    REQUIRE(static_cast<uint8_t>(DistributionShape::UNKNOWN) == 6);
}

TEST_CASE("GaussianParams stores mu, sigma, amplitude", "[distribution]") {
    GaussianParams g{};
    g.mu = 0.5f; g.sigma = 0.1f; g.amplitude = 1.0f;
    REQUIRE(g.mu == Catch::Approx(0.5f));
    REQUIRE(g.sigma == Catch::Approx(0.1f));
    REQUIRE(g.amplitude == Catch::Approx(1.0f));
}

TEST_CASE("BimodalParams stores two Gaussian components + mixing ratio", "[distribution]") {
    BimodalParams b{};
    b.comp1 = {0.3f, 0.05f, 0.7f};
    b.comp2 = {0.8f, 0.08f, 0.3f};
    b.mixing_ratio = 0.7f;
    REQUIRE(b.comp1.mu == Catch::Approx(0.3f));
    REQUIRE(b.comp2.mu == Catch::Approx(0.8f));
    REQUIRE(b.mixing_ratio == Catch::Approx(0.7f));
}

TEST_CASE("SkewNormalParams stores mu, sigma, alpha", "[distribution]") {
    SkewNormalParams s{};
    s.mu = 0.5f; s.sigma = 0.1f; s.alpha = -2.0f;
    REQUIRE(s.alpha == Catch::Approx(-2.0f));
}

TEST_CASE("ZDistribution: default shape is UNKNOWN", "[distribution]") {
    ZDistribution z{};
    REQUIRE(z.shape == DistributionShape::UNKNOWN);
    REQUIRE(z.used_nonparametric == false);
}

TEST_CASE("ZDistribution: Gaussian signal extraction", "[distribution]") {
    ZDistribution z{};
    z.shape = DistributionShape::GAUSSIAN;
    z.params.gaussian = {0.42f, 0.03f, 1.0f};
    z.true_signal_estimate = z.params.gaussian.mu;
    z.signal_uncertainty = z.params.gaussian.sigma;
    z.confidence = 0.95f;
    z.r_squared = 0.97f;
    z.aic = -150.0f;
    z.bic = -145.0f;
    REQUIRE(z.true_signal_estimate == Catch::Approx(0.42f));
    REQUIRE(z.signal_uncertainty == Catch::Approx(0.03f));
    REQUIRE(z.confidence == Catch::Approx(0.95f));
}

TEST_CASE("ZDistribution: SPIKE_OUTLIER stores spike info", "[distribution]") {
    ZDistribution z{};
    z.shape = DistributionShape::SPIKE_OUTLIER;
    z.params.spike_main = {0.40f, 0.02f, 1.0f};
    z.spike_value = 0.95f;
    z.spike_frame_index = 37;
    REQUIRE(z.spike_value == Catch::Approx(0.95f));
    REQUIRE(z.spike_frame_index == 37);
    z.true_signal_estimate = z.params.spike_main.mu;
    REQUIRE(z.true_signal_estimate == Catch::Approx(0.40f));
}

TEST_CASE("ZDistribution: KDE fallback fields", "[distribution]") {
    ZDistribution z{};
    z.shape = DistributionShape::UNIFORM;
    z.used_nonparametric = true;
    z.kde_mode = 0.35f;
    z.kde_bandwidth = 0.02f;
    REQUIRE(z.used_nonparametric == true);
    REQUIRE(z.kde_mode == Catch::Approx(0.35f));
    REQUIRE(z.kde_bandwidth == Catch::Approx(0.02f));
}

TEST_CASE("distribution_shape_name returns correct strings", "[distribution]") {
    REQUIRE(distribution_shape_name(DistributionShape::GAUSSIAN) == std::string("GAUSSIAN"));
    REQUIRE(distribution_shape_name(DistributionShape::BIMODAL) == std::string("BIMODAL"));
    REQUIRE(distribution_shape_name(DistributionShape::SKEWED_LOW) == std::string("SKEWED_LOW"));
    REQUIRE(distribution_shape_name(DistributionShape::SKEWED_HIGH) == std::string("SKEWED_HIGH"));
    REQUIRE(distribution_shape_name(DistributionShape::SPIKE_OUTLIER) == std::string("SPIKE_OUTLIER"));
    REQUIRE(distribution_shape_name(DistributionShape::UNIFORM) == std::string("UNIFORM"));
    REQUIRE(distribution_shape_name(DistributionShape::UNKNOWN) == std::string("UNKNOWN"));
}
