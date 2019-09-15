#include <catch.hpp>
#include "resources.hpp"
#include <learningnet/Compressor.hpp>
#include <learningnet/NetworkChecker.hpp>

using namespace learningnet;

void compressNet(LearningNet &net, int expectedN, int expectedM) {
	Compressor comp;
	comp.compress(net);
	CHECK(countNodes(net) == expectedN);
	CHECK(countArcs(net) == expectedM);

	// The network should still be valid.
	NetworkChecker checker(net);
	CHECKED_ELSE(checker.succeeded()) {
		checker.handleFailure();
		net.write();
	};
}

TEST_CASE("Compressor","[compressor]") {
	SECTION("Example Files") {
		for_file("valid", "not_compressable", [](LearningNet &net) {
			compressNet(net, countNodes(net), countArcs(net));
		});

		for_file("valid", "no_condition", [](LearningNet &net) {
			compressNet(net, 1, 0);
		});

		for_file("valid", "condition", [](LearningNet &net) {
			compressNet(net, 2, 2);
		});

		for_file("valid", "conditions_simple", [](LearningNet &net) {
			compressNet(net, 3, 4);
		});
	}
}
