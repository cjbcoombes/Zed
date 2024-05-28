#include "..\utils\utils.h"

namespace argparse {
	const std::string DEFAULT = "";

	struct Option {
		std::string name;
		std::vector<std::string> args;

		Option(const std::string& name) : name(name), args() {}
	};

	struct Command {
		std::string name;
		std::vector<Option> options;

		Command(const std::string& name) : name(name), options() {}
	};

	typedef std::vector<Command> Argset;

	Argset argParse(int argc, const char* args[]);
}