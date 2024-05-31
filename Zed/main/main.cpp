#include "main.h"

#define CHECK_ARGS(str) if (argc - i < 1) {cout << IO_ERR "Not enough arguments for " str IO_NORM IO_END;return 1;}
#define ERR(str) std::cout << IO_ERR str IO_NORM IO_END; return 1

static int commandDefault(const argparse::Command& c, Flags& globalFlags) {
	for (const argparse::Option& o : c.getOptions()) {
		if (o.getName() == "-h" || o.getName() == "--help") {
			std::cout << mainHelp;
			return 0;
		} else if (o.getName() == "-v" || o.getName() == "--version") {
			std::cout << version;
			return 0;
		} else if (o.getName() == "-d" || o.getName() == "--debug") {
			globalFlags.setFlags(Flags::FLAG_DEBUG);
		}
	}
	return 0;
}

static int commandExecute(const argparse::Command& c, const Flags& globalFlags) {
	executor::ExecutorSettings settings;
	settings.flags.setFlags(globalFlags);

	const std::string* inputPath = nullptr;

	char res = 'X';

	for (const argparse::Option& o : c.getOptions()) {
		if (o.getName() == argparse::DEFAULT) {
			if (!o.getArgs().empty()) inputPath = &o.getArgs().front();
		} else if (o.getName() == "-h" || o.getName() == "--help") {
			std::cout << executeHelp;
			return 0;
		} else if (o.getName() == "-d" || o.getName() == "--debug") {
			settings.flags.setFlags(Flags::FLAG_DEBUG);
		} else if (o.getName() == "-i" || o.getName() == "--in") {
			if (!o.getArgs().empty()) {
				inputPath = &o.getArgs().front();
			} else {
				ERR("Option --in is missing an argument");
			}
		} else if (o.getName() == "-s" || o.getName() == "--stacksize") {
			if (o.getArgs().empty()) {
				ERR("Option --stacksize is missing an argument");
			}
			try {
				settings.stackSize = std::stol(o.getArgs().front());
				if (settings.stackSize <= 0) {
					ERR("Invalid stack size");
				}
				if (settings.stackSize < 256 || settings.stackSize > 512000000) {
					if (settings.stackSize < 256) {
						std::cout << "Are you sure you want your stack size to be " << settings.stackSize << " bytes? (That's really small)\n";
					} else {
						std::cout << "Are you sure you want your stack size to be " << settings.stackSize << " bytes? (That's really big - over 512 MB)\n";
					}
					while (true) {
						std::cout << "(Y/N): ";
						std::cin >> res;
						std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
						if (res == 'Y' || res == 'y') {
							break;
						} else if (res == 'N' || res == 'n') {
							ERR("Aborted stack size choice");
						}
					}
				}
			} catch (const std::invalid_argument&) {
				ERR("Invalid stack size");
			} catch (const std::out_of_range&) {
				ERR("Invalid stack size");
			}
		} else if (o.getName() == "-m" || o.getName() == "--memcheck") {
			settings.flags.setFlags(executor::FLAG_CHECK_MEM);
		}
	}

	if (!inputPath) {
		ERR("Missing input path for execution");
	}

	return executor::exec(inputPath->c_str(), settings);
}

static int commandAssemble(const argparse::Command& c, const Flags& globalFlags) {
	assembler::AssemblerSettings settings;
	settings.flags.setFlags(globalFlags);

	const std::string* inputPath = nullptr;
	const std::string* outputPath = nullptr;

	for (const argparse::Option& o : c.getOptions()) {
		if (o.getName() == argparse::DEFAULT) {
			if (o.getArgs().size() > 0) inputPath = &o.getArgs().front();
			if (o.getArgs().size() > 1) outputPath = &o.getArgs().at(1);
		} else if (o.getName() == "-h" || o.getName() == "--help") {
			std::cout << assembleHelp;
			return 0;
		} else if (o.getName() == "-d" || o.getName() == "--debug") {
			settings.flags.setFlags(Flags::FLAG_DEBUG);
		} else if (o.getName() == "-i" || o.getName() == "--in") {
			if (o.getArgs().size() > 0) {
				inputPath = &o.getArgs().front();
			} else {
				ERR("Option --in is missing an argument");
			}
		} else if (o.getName() == "-o" || o.getName() == "--out") {
			if (o.getArgs().size() > 0) {
				outputPath = &o.getArgs().front();
			} else {
				ERR("Option --out is missing an argument");
			}
		}
	}

	if (!inputPath || !outputPath) {
		ERR("Missing input or output path for assembly");
	}

	return assembler::assemble(inputPath->c_str(), outputPath->c_str(), settings);
}

static int commandDisassemble(const argparse::Command& c, const Flags& globalFlags) {
	disassembler::DisassemblerSettings settings;
	settings.flags.setFlags(globalFlags);

	const std::string* inputPath = nullptr;
	const std::string* outputPath = nullptr;

	for (const argparse::Option& o : c.getOptions()) {
		if (o.getName() == argparse::DEFAULT) {
			if (o.getArgs().size() > 0) inputPath = &o.getArgs().front();
			if (o.getArgs().size() > 1) outputPath = &o.getArgs().at(1);
		} else if (o.getName() == "-h" || o.getName() == "--help") {
			std::cout << disassembleHelp;
			return 0;
		} else if (o.getName() == "-d" || o.getName() == "--debug") {
			settings.flags.setFlags(Flags::FLAG_DEBUG);
		} else if (o.getName() == "-i" || o.getName() == "--in") {
			if (o.getArgs().size() > 0) {
				inputPath = &o.getArgs().front();
			} else {
				ERR("Option --in is missing an argument");
			}
		} else if (o.getName() == "-o" || o.getName() == "--out") {
			if (o.getArgs().size() > 0) {
				outputPath = &o.getArgs().front();
			} else {
				ERR("Option --out is missing an argument");
			}
		}
	}

	if (!inputPath || !outputPath) {
		ERR("Missing input or output path for disassembly");
	}

	return disassembler::disassemble(inputPath->c_str(), outputPath->c_str(), settings);
}

static int commandCompile(const argparse::Command& c, const Flags& globalFlags) {
	compiler::CompilerSettings settings;
	settings.flags.setFlags(globalFlags);
	bool debugSpecified = false;

	const std::string* inputPath = nullptr;
	const std::string* outputPath = nullptr;

	for (const argparse::Option& o : c.getOptions()) {
		if (o.getName() == argparse::DEFAULT) {
			if (o.getArgs().size() > 0) inputPath = &o.getArgs().front();
			if (o.getArgs().size() > 1) outputPath = &o.getArgs().at(1);
		} else if (o.getName() == "-h" || o.getName() == "--help") {
			std::cout << compileHelp;
			return 0;
		} else if (o.getName() == "-d" || o.getName() == "--debug") {
			settings.flags.setFlags(Flags::FLAG_DEBUG);

			if (o.getArgs().size() > 0) {
				debugSpecified = true;
				for (const char c : o.getArgs().front()) {
					if (c == 't') {
						settings.flags.setFlags(compiler::FLAG_DEBUG_TOKENIZER);
					} else if (c == 'a') {
						settings.flags.setFlags(compiler::FLAG_DEBUG_AST);
					} else if (c == 'b') {
						settings.flags.setFlags(compiler::FLAG_DEBUG_BYTECODE);
					}
				}
			}
		} else if (o.getName() == "-i" || o.getName() == "--in") {
			if (o.getArgs().size() > 0) {
				inputPath = &o.getArgs().front();
			} else {
				ERR("Option --in is missing an argument");
			}
		} else if (o.getName() == "-o" || o.getName() == "--out") {
			if (o.getArgs().size() > 0) {
				outputPath = &o.getArgs().front();
			} else {
				ERR("Option --out is missing an argument");
			}
		}
	}

	if (!debugSpecified) {
		settings.flags.setFlags(compiler::FLAG_DEBUG_TOKENIZER | compiler::FLAG_DEBUG_AST | compiler::FLAG_DEBUG_BYTECODE);
	}

	if (!inputPath || !outputPath) {
		ERR("Missing input or output path for compilation");
	}

	return compiler::compile(inputPath->c_str(), outputPath->c_str(), settings);
}

int main(const int argc, const char* argv[]) {
	using namespace std;


	cout << IO_NORM;

	int out = 0;

	const std::span<const char*> args(argv, argc);
	argparse::Argset argset = argparse::argParse(args);
	Flags globalFlags;

	for (const argparse::Command& c : argset) {
		if (c.getName() == argparse::DEFAULT) {
			out = commandDefault(c, globalFlags);
		} else if (c.getName() == "/execute" || c.getName() == "/e") {
			out = commandExecute(c, globalFlags);
		} else if (c.getName() == "/assemble" || c.getName() == "/a") {
			out = commandAssemble(c, globalFlags);
		} else if (c.getName() == "/disassemble" || c.getName() == "/d") {
			out = commandDisassemble(c, globalFlags);
		} else if (c.getName() == "/compile" || c.getName() == "/c") {
			out = commandCompile(c, globalFlags);
		} else {
			cout << IO_WARN "Unknown command: " << c.getName() << " " IO_NORM "\n";
		}
		if (out) return out;
	}

	return 0;
}