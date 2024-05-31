#include "argparse.h"

argparse::Option::Option(const std::string& name) : name(name), args() {}

const std::vector<std::string>& argparse::Option::getArgs() const noexcept { return args; }

const std::string& argparse::Option::getName() const noexcept { return name; }

argparse::Command::Command(const std::string& name) : name(name), options() {}

const std::vector<argparse::Option>& argparse::Command::getOptions() const noexcept { return options; }

const std::string& argparse::Command::getName() const noexcept { return name; }

argparse::Argset argparse::argParse(const std::span<const char*>& args) {
	std::vector<std::string> strs;

	for (const char* arg : args) {
		std::string&& str(arg);
		if (!str.empty()) {
			strs.push_back(std::move(str));
		}
	}

	auto carg = strs.cbegin();
	const auto end = strs.cend();

	// skip exe file name, such as "zed.exe"
	if (carg != end) ++carg;

	Argset argset;
	argset.emplace_back(DEFAULT);
	argset.back().options.emplace_back(DEFAULT);

	while (carg != end) {
		if (carg->at(0) == '/') {
			argset.emplace_back(*carg);
			argset.back().options.emplace_back(DEFAULT);
		} else if (carg->at(0) == '-') {
			argset.back().options.emplace_back(*carg);
		} else {
			argset.back().options.back().args.emplace_back(*carg);
		}

		++carg;
	}

	return argset;
}