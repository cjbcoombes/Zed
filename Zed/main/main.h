#pragma once
#include "..\utils\utils.h"
#include "..\assembler\assembler.h"
#include "..\disassembler\disassembler.h"
#include "..\vm\executor.h"
#include "..\compiler\compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Main

// The main function
int main(int argc, const char* args[]);

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