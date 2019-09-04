#include <catch.hpp>
#include <learningnet/Compressor.hpp>

using namespace learningnet;

TEST_CASE("Compressor","[compressor]") {
	SECTION("base cases") {
		Compressor comp;
		CHECK(1 == 1);
	}
}
