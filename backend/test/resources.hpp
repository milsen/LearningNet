#pragma once

#include <catch.hpp>
#include <learningnet/LearningNet.hpp>
#include <experimental/filesystem>
#include <iostream>
#include <fstream>

using namespace learningnet;

const std::string resourcePath = "../test/resources/";

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
	for (const auto &entry : std::experimental::filesystem::directory_iterator(resourcePath + subdir)) {
		testFile(entry.path(), func, faultyNetwork);
	}
}

void for_file(const std::string &subdir, const std::string &filename,
		const std::function<void(LearningNet&)> &func,
		bool faultyNetwork = false)
{
	testFile(resourcePath + subdir + "/" + filename + ".lgf", func, faultyNetwork);
}
