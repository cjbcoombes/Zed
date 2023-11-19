#include "main.h"

#define CHECK_ARGS(str) if (argc - i < 1) {cout << IO_ERR "Not enough arguments for " str IO_NORM IO_END;return 1;}
int main(int argc, const char* args[]) {
	using namespace std;

	cout << IO_NORM;

	// cout << args[0] << "\n";

	assembler::AssemblerSettings asmSettings;
	disassembler::DisassemblerSettings disasmSettings;
	executor::ExecutorSettings execSettings;
	compiler::CompilerSettings compSettings;

	int out = 0;
	char* dummy = nullptr;
	char res = 'X';

	for (int clarg, i = 1; i < argc; i++) {
		clarg = lookupString(args[i], clargStrings, clargCount);
		if (clarg == -1) {
			cout << IO_WARN "Unknown command line argument: " << args[i] << " " IO_NORM "\n";
			break;
		}
		switch (static_cast<CLArg>(clarg)) {
			case CLArg::DEBUG:
				asmSettings.flags.setFlags(Flags::FLAG_DEBUG);
				execSettings.flags.setFlags(Flags::FLAG_DEBUG);
				disasmSettings.flags.setFlags(Flags::FLAG_DEBUG);
				compSettings.flags.setFlags(Flags::FLAG_DEBUG);
				break;
			case CLArg::NODEBUG:
				asmSettings.flags.unsetFlags(Flags::FLAG_DEBUG);
				execSettings.flags.unsetFlags(Flags::FLAG_DEBUG);
				disasmSettings.flags.unsetFlags(Flags::FLAG_DEBUG);
				compSettings.flags.unsetFlags(Flags::FLAG_DEBUG);
				break;
			case CLArg::ASSEMBLE:
				i += 2;
				CHECK_ARGS("assembler");
				out = assembler::assemble(args[i - 1], args[i], asmSettings);
				break;
			case CLArg::DISASSEMBLE:
				i += 2;
				CHECK_ARGS("disassembler");
				out = disassembler::disassemble(args[i - 1], args[i], disasmSettings);
				break;
			case CLArg::EXECUTE:
				i++;
				CHECK_ARGS("executor");
				out = executor::exec(args[i], execSettings);
				break;
			case CLArg::COMPILE:
				i += 2;
				CHECK_ARGS("compiler");
				out = compiler::compile(args[i - 1], args[i], compSettings);
				break;
			case CLArg::STACK_SIZE:
				i++;
				CHECK_ARGS("setting stack size");
				try {
					execSettings.stackSize = std::strtoul(args[i], &dummy, 0);
					if (execSettings.stackSize < 256 || execSettings.stackSize > 512000000) {
						if (execSettings.stackSize < 256) {
							cout << "Are you sure you want your stack size to be " << execSettings.stackSize << " bytes? (That's really small)\n";
						} else {
							cout << "Are you sure you want your stack size to be " << execSettings.stackSize << " bytes? (That's really big - over 512 MB)\n";
						}
						while (true) {
							cout << "(Y/N): ";
							cin >> res;
							cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
							if (res == 'Y' || res == 'y') {
								break;
							} else if (res == 'N' || res == 'n') {
								out = 1;
								break;
							}
						}
					}
				} catch (std::exception&) {
					cout << IO_ERR "Invalid stack size" IO_NORM "\n";
					out = 1;
				}
				break;
		}
		if (out) {
			return 1;
		}
	}

	return 0;
}