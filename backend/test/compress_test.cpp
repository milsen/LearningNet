#include <catch.hpp>
#include "resources.hpp"
#include <learningnet/Compressor.hpp>

using namespace learningnet;

void compressNet(LearningNet &net, int expectedN, int expectedM) {
	Compressor comp;
	comp.compress(net);
	CHECK(countNodes(net) == expectedN);
	CHECK(countArcs(net) == expectedM);
}

TEST_CASE("Compressor","[compressor]") {
	SECTION("Example Files") {
		for_file("valid", "not_compressable", [](LearningNet &net) {
			compressNet(net, countNodes(net), countArcs(net));
		});

		for_file("valid", "no_condition", [](LearningNet &net) {
			compressNet(net, 1, 0);
		});
	}
}
