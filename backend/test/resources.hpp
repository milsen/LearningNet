#pragma once

#include <catch.hpp>
#include <learningnet/LearningNet.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>

using namespace learningnet;

void testFile(const std::string &filename,
		const std::function<void(LearningNet&)> &func,
		bool faultyNetwork = false)
{
	SECTION(filename) {
		std::ifstream f(filename);
		CHECK(f);

		std::ostringstream netss;
		netss << f.rdbuf();
		if (faultyNetwork) {
			CHECK_THROWS([&netss](){ LearningNet net{netss.str()}; }());
		} else {
			LearningNet net{netss.str()};
			func(net);
		}
	}
}

void for_each_file(const std::string &subdir,
		const std::function<void(LearningNet&)> &func,
		bool faultyNetwork = false)
{
    const std::string resourcePath = "../test/resources/";
	for (const auto &entry : std::filesystem::directory_iterator(resourcePath + subdir)) {
		testFile(entry.path(), func, faultyNetwork);
	}
}
