#include "catch_amalgamated.hpp"
#include "stretch_auto_selector.hpp"
#include "filter_classifier.hpp"
#include "nukex/stretch/veralux_stretch.hpp"

using namespace nukex;

TEST_CASE("select_auto: LRGB_MONO picks VeraLux and logs class name", "[module][auto_selector]") {
    auto sel = select_auto(FilterClass::LRGB_MONO);
    REQUIRE(sel.op != nullptr);
    REQUIRE(dynamic_cast<VeraLuxStretch*>(sel.op.get()) != nullptr);
    REQUIRE(sel.log_line.find("LRGB-mono") != std::string::npos);
    REQUIRE(sel.log_line.find("VeraLux") != std::string::npos);
}

TEST_CASE("select_auto: all classes produce non-null op + non-empty log", "[module][auto_selector]") {
    for (FilterClass c : {FilterClass::LRGB_MONO, FilterClass::LRGB_COLOR,
                          FilterClass::BAYER_RGB, FilterClass::NARROWBAND}) {
        auto sel = select_auto(c);
        REQUIRE(sel.op != nullptr);
        REQUIRE(!sel.log_line.empty());
    }
}
