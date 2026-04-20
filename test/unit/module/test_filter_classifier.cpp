#include "catch_amalgamated.hpp"
#include "filter_classifier.hpp"

using nukex::FITSMetadata;
using nukex::FilterClass;
using nukex::classify_filter;

TEST_CASE("classify_filter: NAXIS3=3 no BAYERPAT -> LRGB_COLOR", "[module][filter_classifier]") {
    FITSMetadata m;
    m.naxis3 = 3;
    REQUIRE(classify_filter(m) == FilterClass::LRGB_COLOR);
}

TEST_CASE("classify_filter: BAYERPAT present -> BAYER_RGB", "[module][filter_classifier]") {
    FITSMetadata m;
    m.naxis3 = 1;
    m.bayer_pat = "RGGB";
    REQUIRE(classify_filter(m) == FilterClass::BAYER_RGB);
}

TEST_CASE("classify_filter: narrowband names -> NARROWBAND", "[module][filter_classifier]") {
    for (const std::string& name : {"HA", "H-ALPHA", "HALPHA", "OIII", "O3", "SII", "S2", "NARROWBAND"}) {
        FITSMetadata m;
        m.filter = name;
        REQUIRE(classify_filter(m) == FilterClass::NARROWBAND);
    }
}

TEST_CASE("classify_filter: mono LRGB filters -> LRGB_MONO", "[module][filter_classifier]") {
    for (const std::string& name : {"L", "LUM", "R", "G", "B", "RED", "GREEN", "BLUE"}) {
        FITSMetadata m;
        m.filter = name;
        REQUIRE(classify_filter(m) == FilterClass::LRGB_MONO);
    }
}

TEST_CASE("classify_filter: empty metadata -> LRGB_MONO (safe default)",
          "[module][filter_classifier]") {
    FITSMetadata m;
    REQUIRE(classify_filter(m) == FilterClass::LRGB_MONO);
}

TEST_CASE("classify_filter: BAYERPAT wins over NAXIS3=3",
          "[module][filter_classifier]") {
    FITSMetadata m;
    m.naxis3 = 3;
    m.bayer_pat = "RGGB";
    REQUIRE(classify_filter(m) == FilterClass::BAYER_RGB);
}
