#include <catch.hpp>
#include "resources.hpp"
#include <learningnet/Compressor.hpp>
#include <learningnet/NetworkChecker.hpp>

using namespace learningnet;

void compressNet(LearningNet &net,
	int expectedN,
	int expectedM,
	TargetReachability expectedResult = TargetReachability::Unknown)
{
	Compressor comp{net};
	CHECK(comp.getResult() == expectedResult);
	CHECK(countNodes(net) == expectedN);
	CHECKED_ELSE(countArcs(net) == expectedM);

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
			compressNet(net, 1, 0, TargetReachability::Yes);
		});

		for_file("valid", "condition", [](LearningNet &net) {
			compressNet(net, 1, 0, TargetReachability::Yes);
		});

		for_file("valid", "condition_simple", [](LearningNet &net) {
			compressNet(net, 2, 2, TargetReachability::Yes);
		});

		for_file("valid", "conditions_simple", [](LearningNet &net) {
			compressNet(net, 3, 4);
		});

		for_file("valid", "pre_top_sort", [](LearningNet &net) {
			compressNet(net, 2, 2, TargetReachability::Yes);
		});

		for_file("valid", "pre_top_sort_2", [](LearningNet &net) {
			compressNet(net, 10, 15);
		});

		for_file("valid", "split_to_join", [](LearningNet &net) {
			compressNet(net, 2, 2, TargetReachability::Yes);
		});

		// The following nets could probably be compressed further with better
		// algorithmic ideas.
		for_file("valid", "split_partial_to_join", [](LearningNet &net) {
			compressNet(net, 3, 5, TargetReachability::Unknown);
		});

		for_file("valid", "condition_partial_to_join", [](LearningNet &net) {
			compressNet(net, 2, 3, TargetReachability::Unknown);
		});
	}
}
