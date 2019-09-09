#include <catch.hpp>
#include <learningnet/NetworkChecker.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace learningnet;

void testFile(const std::string &filename,
		const std::function<void(LearningNet&)> &func) {
	SECTION(filename) {
		std::ifstream f(filename);
		CHECK(f);

		std::ostringstream netss;
		netss << f.rdbuf();
		LearningNet net{netss.str()};
		func(net);
	}
}

void testFile(const std::string &filename, bool valid) {
	testFile(filename, [&valid](LearningNet &net) {
		NetworkChecker checker(net);
		CHECK(checker.succeeded() == valid);
	});
}

TEST_CASE("NetworkChecker","[check]") {
    const std::string resourcePath = "../test/resources/";

	SECTION("Example Files") {
		for (const auto &entry : std::filesystem::directory_iterator(resourcePath + "valid/")) {
			testFile(entry.path(), true);
		}

		for (const auto &entry : std::filesystem::directory_iterator(resourcePath + "invalid/")) {
			testFile(entry.path(), false);
		}
	}
}
