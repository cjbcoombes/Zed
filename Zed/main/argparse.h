#include "..\utils\utils.h"

namespace argparse {
	const std::string DEFAULT = "";

	struct Command;
	typedef std::vector<Command> Argset;

	Argset argParse(const std::span<const char*>& args);

	struct Option {
		friend Argset argparse::argParse(const std::span<const char*>& args);

	private:
		const std::string name;
		std::vector<std::string> args;

	public:
		Option(const std::string& name);

		const std::vector<std::string>& getArgs() const noexcept;
		const std::string& getName() const noexcept;
	};

	struct Command {
		friend Argset argparse::argParse(const std::span<const char*>& args);

	private:
		const std::string name;
		std::vector<Option> options;

	public:
		Command(const std::string& name);

		const std::vector<argparse::Option>& getOptions() const noexcept;
		const std::string& getName() const noexcept;
	};
}