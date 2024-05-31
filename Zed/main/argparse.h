#include "../utils/utils.h"

namespace argparse {
	const std::string DEFAULT = "";

	struct Command;
	typedef std::vector<Command> Argset;

	// Parses args into a list of commands
	Argset argParse(const std::span<const char*>& args);

	// An option is like "--name arg1 arg2 ..."
	struct Option {
		friend Argset argparse::argParse(const std::span<const char*>& args);

	private:
		const std::string name;
		std::vector<std::string> args;

	public:
		explicit Option(const std::string& name);

		const std::vector<std::string>& getArgs() const noexcept;
		const std::string& getName() const noexcept;
	};

	// A command is like "/name arg1 --option optarg1 ..."
	struct Command {
		friend Argset argparse::argParse(const std::span<const char*>& args);

	private:
		const std::string name;
		std::vector<Option> options;

	public:
		explicit Command(const std::string& name);

		const std::vector<argparse::Option>& getOptions() const noexcept;
		const std::string& getName() const noexcept;
	};
}