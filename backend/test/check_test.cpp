#include <catch.hpp>
#include "resources.hpp"
#include <learningnet/NetworkChecker.hpp>

using namespace learningnet;

void checkNet(LearningNet &net, bool valid) {
	NetworkChecker checker(net);
	CHECK(checker.succeeded() == valid);
}

TEST_CASE("NetworkChecker","[check]") {
	SECTION("Example Files") {
		for_each_file("valid", [](LearningNet &net) {
			checkNet(net, true);
		});

		for_each_file("invalid", [](LearningNet &net) {
			checkNet(net, false);
		});

		for_each_file("exception", [](LearningNet &net) {
			checkNet(net, false);
		}, true);
	}
}
