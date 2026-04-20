#include "catch_amalgamated.hpp"
#include "stretch_factory.hpp"
#include "fits_metadata.hpp"
#include "nukex/stretch/veralux_stretch.hpp"
#include "nukex/stretch/ghs_stretch.hpp"
#include "nukex/stretch/mtf_stretch.hpp"
#include "nukex/stretch/arcsinh_stretch.hpp"
#include "nukex/stretch/log_stretch.hpp"
#include "nukex/stretch/lupton_stretch.hpp"
#include "nukex/stretch/clahe_stretch.hpp"

using namespace nukex;

TEST_CASE("build_primary: Auto fills log_line", "[module][stretch_factory]") {
    FITSMetadata meta;
    meta.filter = "L";
    std::string log;
    auto op = build_primary(PrimaryStretch::Auto, meta, log);
    REQUIRE(op != nullptr);
    REQUIRE(!log.empty());
}

TEST_CASE("build_primary: explicit values return empty log_line", "[module][stretch_factory]") {
    FITSMetadata meta;
    std::string log = "prior";
    auto op = build_primary(PrimaryStretch::GHS, meta, log);
    REQUIRE(op != nullptr);
    REQUIRE(log.empty());
    REQUIRE(dynamic_cast<GHSStretch*>(op.get()) != nullptr);
}

TEST_CASE("build_primary: all named enums produce the correct op type", "[module][stretch_factory]") {
    FITSMetadata meta;
    std::string log;
    REQUIRE(dynamic_cast<VeraLuxStretch*>(build_primary(PrimaryStretch::VeraLux, meta, log).get()) != nullptr);
    REQUIRE(dynamic_cast<GHSStretch*>    (build_primary(PrimaryStretch::GHS,     meta, log).get()) != nullptr);
    REQUIRE(dynamic_cast<MTFStretch*>    (build_primary(PrimaryStretch::MTF,     meta, log).get()) != nullptr);
    REQUIRE(dynamic_cast<ArcSinhStretch*>(build_primary(PrimaryStretch::ArcSinh, meta, log).get()) != nullptr);
    REQUIRE(dynamic_cast<LogStretch*>    (build_primary(PrimaryStretch::Log,     meta, log).get()) != nullptr);
    REQUIRE(dynamic_cast<LuptonStretch*> (build_primary(PrimaryStretch::Lupton,  meta, log).get()) != nullptr);
    REQUIRE(dynamic_cast<CLAHEStretch*>  (build_primary(PrimaryStretch::CLAHE,   meta, log).get()) != nullptr);
}

TEST_CASE("build_finishing: None returns nullptr", "[module][stretch_factory]") {
    REQUIRE(build_finishing(FinishingStretch::None) == nullptr);
}
