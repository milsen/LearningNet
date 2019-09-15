#include <catch.hpp>
#include "resources.hpp"
#include <learningnet/NetworkChecker.hpp>

using namespace learningnet;

void checkNet(LearningNet &net, bool valid, bool useCompression) {
	NetworkChecker checker(net, useCompression);
	CHECKED_ELSE(checker.succeeded() == valid) {
		checker.handleFailure();
		net.write();
	}
}

TEST_CASE("NetworkChecker","[check]") {
	for (bool useCompression : {false, true}) {
		std::string compressionStr = useCompression ? "with" : "without";

		SECTION(compressionStr + " compression") {
			for_each_file("valid", [&](LearningNet &net) {
				checkNet(net, true, useCompression);
			});

			for_each_file("invalid", [&](LearningNet &net) {
				checkNet(net, false, useCompression);
			});

			for_each_file("exception", [&](LearningNet &net) {
				checkNet(net, false, useCompression);
			}, true);
		}
	}
}
