#include "argparse.h"

argparse::Argset argparse::argParse(int argc, const char* args[]) {
	std::vector<std::string> strs;

	for (; argc > 0; argc--) {
		std::string&& str = args[0];
		if (str.size() != 0) {
			strs.push_back(std::move(str));
		}
		args++;
	}

	auto carg = strs.cbegin();
	const auto end = strs.cend();
	
	// skip exe file name, such as "zed.exe"
	if (carg != end) carg++;

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

		carg++;
	}

	return argset;
}