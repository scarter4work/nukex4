#include "catch_amalgamated.hpp"
#include "nukex/stretch/param_model.hpp"
#include "nukex/stretch/veralux_stretch.hpp"

#include <limits>

using namespace nukex;

namespace {
ImageStats trivial_stats() {
    ImageStats s;
    s.median.fill(0.5); s.mad.fill(0.1);
    s.p50.fill(0.5);    s.p95.fill(0.9); s.p99.fill(0.95); s.p999.fill(0.99);
    s.skew.fill(0.0);   s.sat_frac.fill(0.0);
    s.bright_concentration = 0.2;
    s.color_rg = 1.0; s.color_bg = 1.0;
    s.fwhm_median = 3.0; s.star_count = 100;
    return s;
}
} // namespace

TEST_CASE("ParamModel: apply with zero-coefficient predicts intercept",
          "[stretch][param_model]") {
    ParamModel m("VeraLux");
    ParamCoefficients c;
    c.feature_mean.assign(29, 0.5);
    c.feature_std .assign(29, 1.0);
    c.coefficients.assign(29, 0.0);
    c.intercept = 3.5;
    m.add_param("log_D", c);

    VeraLuxStretch op;
    REQUIRE(m.predict_and_apply(trivial_stats(), op));
    REQUIRE(op.get_param("log_D").value() == Catch::Approx(3.5f));
}

TEST_CASE("ParamModel: clamps prediction to param_bounds",
          "[stretch][param_model]") {
    ParamModel m("VeraLux");
    ParamCoefficients c;
    c.feature_mean.assign(29, 0.0);
    c.feature_std .assign(29, 1.0);
    c.coefficients.assign(29, 0.0);
    c.intercept = 99.0;   // above VeraLux log_D max of 7.0
    m.add_param("log_D", c);

    VeraLuxStretch op;
    REQUIRE(m.predict_and_apply(trivial_stats(), op));
    REQUIRE(op.get_param("log_D").value() == Catch::Approx(7.0f));
}

TEST_CASE("ParamModel: drops unknown param names",
          "[stretch][param_model]") {
    ParamModel m("VeraLux");
    ParamCoefficients c;
    c.feature_mean.assign(29, 0.0);
    c.feature_std .assign(29, 1.0);
    c.coefficients.assign(29, 0.0);
    c.intercept = 1.0;
    m.add_param("bogus_param_name", c);

    VeraLuxStretch op;
    // Nothing applied because VeraLux has no "bogus_param_name". Not an error.
    REQUIRE_FALSE(m.predict_and_apply(trivial_stats(), op));
    REQUIRE(op.log_D == 2.0f);  // default unchanged
}

TEST_CASE("ParamModel: NaN stats row falls through",
          "[stretch][param_model]") {
    ParamModel m("VeraLux");
    ParamCoefficients c;
    c.feature_mean.assign(29, 0.0);
    c.feature_std .assign(29, 1.0);
    c.coefficients.assign(29, 0.5);
    c.intercept = 1.0;
    m.add_param("log_D", c);

    ImageStats s = trivial_stats();
    s.median[0] = std::numeric_limits<double>::quiet_NaN();

    VeraLuxStretch op;
    // Non-finite input -> model refuses to predict -> false. Op unchanged.
    REQUIRE_FALSE(m.predict_and_apply(s, op));
    REQUIRE(op.log_D == 2.0f);
}
