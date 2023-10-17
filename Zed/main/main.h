#pragma once
#include "..\utils\utils.h"
#include "..\assembler\assembler.h"

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

	Count
};

constexpr int clargCount = static_cast<int>(CLArg::Count);
constexpr const char* clargStrings[] = {
	"-debug",
	"-nodebug",
	"-assemble",
	"-disassemble",
	"-execute",
	"-compile"
};