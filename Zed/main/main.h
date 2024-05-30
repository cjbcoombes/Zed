#pragma once
#include "../utils/utils.h"
#include "../assembler/assembler.h"
#include "../disassembler/disassembler.h"
#include "../vm/executor.h"
#include "../compiler/compiler.h"
#include "argparse.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Main

// The main function
int main(const int argc, const char* args[]);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Help Menus

constexpr const char* version = "Zed. No official versions yet\n";

constexpr const char* mainHelp =
"Zed Help\n"
"========\n"
"Commands/Options:\n"
"    -h, --help          display this help information\n"
"    -d, --debug         set debug mode for all following commands\n"
"    /e, /execute        execute a .eze executable\n"
"    /a, /assemble       assemble a .azm file into a .eze executable\n"
"    /d, /disassemble    disassemble a .eze executable\n"
"    /c, /compile        compile a .z file into a .eze executable\n"
"\n"
"For specific command help, use the help option under the command.\n"
"    Example: zed.exe /compile --help\n";

constexpr const char* executeHelp = "TODO\n";
constexpr const char* assembleHelp = "TODO\n";
constexpr const char* disassembleHelp = "TODO\n";
constexpr const char* compileHelp = "TODO\n";

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Command Line Arguments

// The possible cli args
enum class CLArg {
	DEBUG,
	NODEBUG,
	ASSEMBLE,
	DISASSEMBLE,
	EXECUTE,
	COMPILE,
	STACK_SIZE,

	Count
};

constexpr int clargCount = static_cast<int>(CLArg::Count);
constexpr const char* clargStrings[] = {
	"--debug",
	"--nodebug",
	"--assemble",
	"--disassemble",
	"--execute",
	"--compile",
	"--stacksize"
};