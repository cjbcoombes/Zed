#include "main.h"

#define CHECK_ARGS(str) if (argc - i < 1) {cout << IO_ERR "Not enough arguments for " str IO_NORM IO_END;return 1;}
#define ERR(str) std::cout << IO_ERR str IO_NORM IO_END; return 1

int commandDefault(argparse::Command& c, Flags & globalFlags) {
	for (argparse::Option& o : c.options) {
		if (o.name == "-h" || o.name == "--help") {
			std::cout << mainHelp;
			return 0;
		} else if (o.name == "-v" || o.name == "--version") {
			std::cout << version;
			return 0;
		} else if (o.name == "-d" || o.name == "--debug") {
			globalFlags.setFlags(Flags::FLAG_DEBUG);
		}
	}
	return 0;
}

int commandExecute(argparse::Command& c, Flags& globalFlags) {
	executor::ExecutorSettings settings;
	settings.flags.setFlags(globalFlags.bits);

	std::string* inputPath = nullptr;

	char* dummy = nullptr;
	char res = 'X';

	for (argparse::Option& o : c.options) {
		if (o.name == argparse::DEFAULT) {
			if (o.args.size() > 0) inputPath = &o.args[0];
		} else if (o.name == "-h" || o.name == "--help") {
			std::cout << executeHelp;
			return 0;
		} else if (o.name == "-d" || o.name == "--debug") {
			settings.flags.setFlags(Flags::FLAG_DEBUG);
		} else if (o.name == "-i" || o.name == "--in") {
			if (o.args.size() > 0) {
				inputPath = &o.args[0];
			} else {
				ERR("Option --in is missing an argument");
			}
		} else if (o.name == "-s" || o.name == "--stacksize") {
			if (o.args.size() < 1) {
				ERR("Option --stacksize is missing an argument");
			}
			try {
				settings.stackSize = std::stoul(o.args[0]);
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
		}
	}

	if (!inputPath) {
		ERR("Missing input path for execution");
	}

	return executor::exec(inputPath->c_str(), settings);
}

int commandAssemble(argparse::Command& c, Flags& globalFlags) {
	assembler::AssemblerSettings settings;
	settings.flags.setFlags(globalFlags.bits);

	std::string* inputPath = nullptr;
	std::string* outputPath = nullptr;

	for (argparse::Option& o : c.options) {
		if (o.name == argparse::DEFAULT) {
			if (o.args.size() > 0) inputPath = &o.args[0];
			if (o.args.size() > 1) outputPath = &o.args[1];
		} else if (o.name == "-h" || o.name == "--help") {
			std::cout << assembleHelp;
			return 0;
		} else if (o.name == "-d" || o.name == "--debug") {
			settings.flags.setFlags(Flags::FLAG_DEBUG);
		} else if (o.name == "-i" || o.name == "--in") {
			if (o.args.size() > 0) {
				inputPath = &o.args[0];
			} else {
				ERR("Option --in is missing an argument");
			}
		} else if (o.name == "-o" || o.name == "--out") {
			if (o.args.size() > 0) {
				outputPath = &o.args[0];
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

int commandDisassemble(argparse::Command& c, Flags& globalFlags) {
	disassembler::DisassemblerSettings settings;
	settings.flags.setFlags(globalFlags.bits);

	std::string* inputPath = nullptr;
	std::string* outputPath = nullptr;

	for (argparse::Option& o : c.options) {
		if (o.name == argparse::DEFAULT) {
			if (o.args.size() > 0) inputPath = &o.args[0];
			if (o.args.size() > 1) outputPath = &o.args[1];
		} else if (o.name == "-h" || o.name == "--help") {
			std::cout << disassembleHelp;
			return 0;
		} else if (o.name == "-d" || o.name == "--debug") {
			settings.flags.setFlags(Flags::FLAG_DEBUG);
		} else if (o.name == "-i" || o.name == "--in") {
			if (o.args.size() > 0) {
				inputPath = &o.args[0];
			} else {
				ERR("Option --in is missing an argument");
			}
		} else if (o.name == "-o" || o.name == "--out") {
			if (o.args.size() > 0) {
				outputPath = &o.args[0];
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

int commandCompile(argparse::Command& c, Flags& globalFlags) {
	compiler::CompilerSettings settings;
	settings.flags.setFlags(globalFlags.bits);
	bool debugSpecified = false;

	std::string* inputPath = nullptr;
	std::string* outputPath = nullptr;

	for (argparse::Option& o : c.options) {
		if (o.name == argparse::DEFAULT) {
			if (o.args.size() > 0) inputPath = &o.args[0];
			if (o.args.size() > 1) outputPath = &o.args[1];
		} else if (o.name == "-h" || o.name == "--help") {
			std::cout << compileHelp;
			return 0;
		} else if (o.name == "-d" || o.name == "--debug") {
			settings.flags.setFlags(Flags::FLAG_DEBUG);

			if (o.args.size() > 0) {
				debugSpecified = true;
				for (char c : o.args[0]) {
					if (c == 't') {
						settings.flags.setFlags(compiler::FLAG_DEBUG_TOKENIZER);
					} else if (c == 'a') {
						settings.flags.setFlags(compiler::FLAG_DEBUG_AST);
					} else if (c == 'b') {
						settings.flags.setFlags(compiler::FLAG_DEBUG_BYTECODE);
					}
				}
			}
		} else if (o.name == "-i" || o.name == "--in") {
			if (o.args.size() > 0) {
				inputPath = &o.args[0];
			} else {
				ERR("Option --in is missing an argument");
			}
		} else if (o.name == "-o" || o.name == "--out") {
			if (o.args.size() > 0) {
				outputPath = &o.args[0];
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

int main(int argc, const char* args[]) {
	using namespace std;

	cout << IO_NORM;

	int out = 0;

	argparse::Argset argset = argparse::argParse(argc, args);
	Flags globalFlags;

	for (argparse::Command& c : argset) {
		if (c.name == argparse::DEFAULT) {
			out = commandDefault(c, globalFlags);
		} else if (c.name == "/execute" || c.name == "/e") {
			out = commandExecute(c, globalFlags);
		} else if (c.name == "/assemble" || c.name == "/a") {
			out = commandAssemble(c, globalFlags);
		} else if (c.name == "/disassemble" || c.name == "/d") {
			out = commandDisassemble(c, globalFlags);
		} else if (c.name == "/compile" || c.name == "/c") {
			out = commandCompile(c, globalFlags);
		} else {
			cout << IO_WARN "Unknown command: " << c.name << " " IO_NORM "\n";
		}
		if (out) return out;
	}

	return 0;
}