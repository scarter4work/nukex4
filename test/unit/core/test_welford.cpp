#include "catch_amalgamated.hpp"
#include "nukex/core/types.hpp"

TEST_CASE("Build system smoke test", "[build]") {
    REQUIRE(nukex::MAX_CHANNELS == 8);
}
